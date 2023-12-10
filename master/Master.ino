#include <SoftwareSerial.h>
#include "SNIPE.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "seatStatusSender.h"   //와이파이로 서버에 http put하는 헤더

// 와이파이 정보, http 설정
const char *ssid = "SangWon";
const char *password = "11111111";
const char *server = "144.24.90.181";
const String resourcePath = "/iot-comm/ws/fallback/seats";


#define TXpin 11
#define RXpin 10
#define ATSerial Serial

//  로라통신 설정
SoftwareSerial DebugSerial(RXpin,TXpin);
SNIPE SNIPE(ATSerial);

void sendSeatStatus();


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
      
  DebugSerial.println("SNIPE LoRa Master");
  
  setupWiFi();  //와이파이 

}

void loop() {
        String msg = SNIPE.lora_recv();    //마스터가 받기
        
        int id;
        bool state;
        int spaceIndex = msg.indexOf(' ');
        
        String firstStr;
        String secondStr;
        String reponse;
        firstStr = msg.substring(0, spaceIndex)
        secondStr = msg.substring(spaceIndex + 1)

        id = firstStr.toInt()
        state = secondStr == String("true")

        //list 만들어서 서버에 전송해야함 http 통신 규약: https://github.com/IoTComm/LibraEnd/blob/master/httpPolling.md

        repsponse = sendSeatStatus(list);  //list put으로 보내고 서버에서 받은 데이터 response 에 저장

        /*  reponse 로 받은 데이터 로라통신으로 아두이노 들에게 전달 데이터가공   */
        //데이터 가공(reponse);
        
        SNIPE.lora_send(reponse);

        
        
      
        delay(1000);
}
