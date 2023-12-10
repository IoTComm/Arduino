#include <SoftwareSerial.h>
#include "SNIPE.h"

#define TXpin 11
#define RXpin 10
#define ATSerial Serial
#define ID  1   //좌석 번호

SoftwareSerial DebugSerial(RXpin,TXpin);
SNIPE SNIPE(ATSerial);

//16byte hex key
String lora_app_key = "11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 00";

void setup() {
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
  if (!SNIPE.lora_setFreq(LORA_CH_1)) {
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
      
  DebugSerial.println("SNIPE LoRa Actuator");

  // initialize digital pin digital motor & lcd
  
}

void loop() {
        String ver = SNIPE.lora_recv(); //"0 1 0, 0 2 1, 2 1 1" 이런식의 데이터

        
        /*  예시  */
        if ( "seat state change 1 >> 잠금 해제" )
        {
          //모터 작동시켜서 의자 잠금해제, lcd "이용중"
        }
        else if("seat state change 0>> 사용종료")
        {
          //모터 작동 의자 락, lcd "빈자리"
        }
        else if ("강제퇴실시")
        {
          //lcd에 강제퇴실조치 되셨습니다 띄우기     
        }
        else if ("데스크 off시")
        {
          //불끄기 
        }
        delay(1000);
}
