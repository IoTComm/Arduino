#include <SoftwareSerial.h>
#include "SNIPE.h" 

//기본값 설정
#define TRIG 12 //TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO 8 //ECHO 핀 설정 (초음파 받는 핀)
#define ID 1 //이 아두이노 기기의 좌석 번호

int SOUND = A0; //db측정 센서
int db; //측정된 db

float cycletime;
float distance;
bool nowstate; //자리 착석 유무
bool prestate; //state before sensing

//로라 설정
#define TXpin 11
#define RXpin 10
#define ATSerial Serial

SoftwareSerial DebugSerial(RXpin,TXpin);
SNIPE SNIPE(ATSerial);
//16byte hex key
String lora_app_key = "11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 42";  


String makeJson(int id, bool nowstate)
{
  return "{\"seat_id\":"+String(id)+",\"is_active\":"+(nowstate?"true":"false")+"}"
};

String makeMsg(int id, bool state)
{
  return String(id)+" "+nowstate?"true":"false"
}

void setup() {
  /*  센서 설정  */
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  /*  로라 설정  */
  ATSerial.begin(115200);

  // put your setup code here, to run once:
  while(ATSerial.read()>= 0) {}
  while(!ATSerial);

  DebugSerial.begin(115200);

  /* SNIPE LoRa Initialization */
  if (!SNIPE.lora_init()) {
    DebugSerial.println("SNIPE LoRa Initialization Fail!");
    while (1);
  }

  /* SNIPE LoRa Set App Key */
  if (!SNIPE.lora_setAppKey(lora_app_key)) {
    DebugSerial.println("SNIPE LoRa app key value has not been changed");
  }
  
  /* SNIPE LoRa Set Frequency */
  if (!SNIPE.lora_setFreq(LORA_CH_8)) {
    DebugSerial.println("SNIPE LoRa Frequency value has not been changed");
  }

  /* SNIPE LoRa Set Spreading Factor */
  if (!SNIPE.lora_setSf(LORA_SF_7)) {
    DebugSerial.println("SNIPE LoRa Sf value has not been changed");
  }

  /* SNIPE LoRa Set Rx Timeout 
   * If you select LORA_SF_12, 
   * RX Timout use a value greater than 5000  
  */
  if (!SNIPE.lora_setRxtout(5000)) {
    DebugSerial.println("SNIPE LoRa Rx Timout value has not been changed");
  }
        
  DebugSerial.println("SNIPE LoRa Send");
  
}

void loop() {
  /*    거리측정    */
  digitalWrite(trig, HIGH);
  delay(10);
  digitalWrite(trig, LOW);
  cycletime = pulseIn(echo, HIGH); 
  distance = ((340 * cycletime) / 10000) / 2; 

  /*    자리 착석 유무    */
  nowstate = distance <= 100;

  // String Json = makeJson(id,nowsate);  //json문자열 생성

  if(nowstate != prestate) //착석유무 변경될시 
  {
    //lora로 state 변경 사항 마스터에게 알림
    
    if (SNIPE.lora_send(makeMsg(id, nowstate)))
            {
                DebugSerial.println("send state success");
            }
            else
            {
                DebugSerial.println("send fail");
                delay(500);
            }
  }

  prestate = nowstate;


}
