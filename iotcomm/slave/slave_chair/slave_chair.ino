#include <SoftwareSerial.h>
#include "SNIPE.h"
#include <Servo.h>

//로라 설정
#define TXpin 11
#define RXpin 10
#define ATSerial Serial
#define D_LIMIT 100
#define C_ID 1

//초음파 기본값 설정
#define TRIG 12 //TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO 8 //ECHO 핀 설정 (초음파 받는 핀)
#define ID 1 //이 아두이노 기기의 좌석 번호
float cycletime;
float distance;
bool nowstate; //자리 착석 유무
bool prestate; //state before sensing

//모터 기본값 설정

Servo myservo;

  

SoftwareSerial DebugSerial(RXpin,TXpin);
SNIPE SNIPE(ATSerial);

//16byte hex key
String lora_app_key = "11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 42";

void setup() {
  float cycletime;
  float distance;
  bool nowstate; //자리 착석 유무
  Serial.begin(115200);

  myservo.attach(7);

  //로라 설정
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
  if (!SNIPE.lora_setFreq(LORA_CH_3)) {
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
      
  DebugSerial.println("SNIPE LoRa Chair");
}

void loop() {
    /*    거리측정    */
  digitalWrite(TRIG, HIGH);
  delay(10);
  digitalWrite(TRIG, LOW);
  cycletime = pulseIn(ECHO, HIGH); 
  distance = ((340 * cycletime) / 10000) / 2; 

  String ans;
  Serial.println(distance);
  /*    자리 착석 유무    */
  nowstate = distance <= D_LIMIT;
  ans = String(nowstate ? '1' : '0');
  //  로라통신
  String ver = SNIPE.lora_recv();
  if (ver == "C 1 1") //로라가 C_1에게 사용 시작
  {
    myservo.write(45);
    SNIPE.lora_send("1");

  }
  else if(ver == "C 1 0")  //모터 잠구기
  {
    myservo.write(0);
    SNIPE.lora_send("1");
  } 

  else if(ver == "C 1 2") //상태확인
  {
    SNIPE.lora_send(ans);
  }

  delay(1000);

}