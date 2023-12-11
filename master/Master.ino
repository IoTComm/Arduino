#include <SoftwareSerial.h>
#include "SNIPE.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "seatStatusSender2.h"   // ver1은 master가 json만들기, ver2 = json sensor가 만들기

#define TXpin 11
#define RXpin 10
#define ATSerial Serial

String convertJsonToMessage(String json) {  //서버로 받아온 데이터 변형시키는 함수
  String result = "";

  // return "ddfadafds";

  int start = 0;
  int end = 0;

  // seat_state_change  
  start = json.indexOf("\"seat_state_change\"");
  if (start != -1) {
    start = json.indexOf("[", start);
    end = json.indexOf("]", start);
    if (end != -1) {
      String seatStateChange = json.substring(start + 1, end);
      int index = 0;
      while (index != -1) {
        result += "0 "; // seat_state_change 타입을 나타내는 '0'을 추가
        index = seatStateChange.indexOf("\"seat_id\":", index);
        if (index != -1) {
          result += seatStateChange.substring(index + 9, seatStateChange.indexOf(",", index));
          result += " "; // seat_id 추가
          index = seatStateChange.indexOf("\"is_active\":", index);
          if (index != -1) {
            // is_active 값에 따라 1 또는 0 추가
            result += (seatStateChange.substring(index + 12, seatStateChange.indexOf("}", index)) == "true" ? "1" : "0");
            result += ","; // 쉼표 추가
            index++;
          }
        }
      }
    }
  }

  // seat_timeout  
  start = json.indexOf("\"seat_timeout\"");
  if (start != -1) {
    start = json.indexOf("[", start);
    end = json.indexOf("]", start);
    if (end != -1) {
      String seatTimeout = json.substring(start + 1, end);
      int index = 0;
      while (index != -1) {
        result += "1 "; // seat_timeout 타입 나타내는 1 추가
        index = seatTimeout.indexOf("\"seat_id\":", index);
        if (index != -1) {
          result += seatTimeout.substring(index + 9, seatTimeout.indexOf(",", index));
          result += " "; // seat_id 추가
          result += ","; // 쉼표 추가
          index++;
        }
      }
    }
  }

  // desk 
  start = json.indexOf("\"desk\"");
  if (start != -1) {
    start = json.indexOf("[", start);
    end = json.indexOf("]", start);
    if (end != -1) {
      String desk = json.substring(start + 1, end);
      int index = 0;
      while (index != -1) {
        result += "2 "; // desk 타입 나타내는 2를 추가
        index = desk.indexOf("\"id\":", index);
        if (index != -1) {
          result += desk.substring(index + 5, desk.indexOf(",", index));
          result += " "; // id 추가
          index = desk.indexOf("\"state\":", index);
          if (index != -1) {
            const char* brace = "{";
            // state 값에 따라 1 또는 0 추가
            // ((index + 8 ==  desk.indexOf(brace, index)) ? "1" : "0");
            result += ","; // 쉼표 추가
            index++;
          }
        }
      }
    }
  }

  // 마지막 쉼표 제거
  if (result.endsWith(",")) {
    result.remove(result.length() - 1);
  }

  return result;
}


String response;

//  로라통신 설정
SoftwareSerial DebugSerial(RXpin,TXpin);
SNIPE SNIPE(ATSerial);


//16byte hex key
String lora_app_key = "11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 42";

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
      
  DebugSerial.println("SNIPE LoRa Master");
  
  /* wifi설정 */
  Serial.begin(115200);
  setupWiFi();

}

void loop() {
        String diffUsing = SNIPE.lora_recv();    //마스터가 센서에게 받는값 (자리 착석 유무 변경사항)
        

        
        response = sendSeatStatus(diffUsing);  //diffusing put으로 보내고 서버에서 받은 데이터 response 에 저장

        /*  reponse 로 받은 데이터 로라통신으로 아두이노 들에게 전달 데이터가공   */
        //데이터 가공(reponse); 0 1 0, 0 2 1, 1 2 1, 2 1 4
        convertJsonToMessage(response);
        
        SNIPE.lora_send(response);   //엑츄에이터에게 전달  엑츄에이터는 의자 잠구고 lcd등에 알림

        
        
      
        delay(1000);
}
