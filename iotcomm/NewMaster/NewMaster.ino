#include <SoftwareSerial.h>
#include "SNIPE.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "seatStatusSender2.h" // ver1은 master가 json만들기, ver2 = json sensor가 만들기

#define TXpin 11
#define RXpin 10
#define ATSerial Serial

#define SEAT_COUNT 1
#define DESK_COUNT 1

#define SECRET_KEY "verysecretadminkey"

String seatCommands[SEAT_COUNT];
String deskCommands[DESK_COUNT];

bool prevSeatState[SEAT_COUNT] = {false};
bool currSeatState[SEAT_COUNT] = {false};

String response;

//  로라통신 설정
SoftwareSerial DebugSerial(RXpin, TXpin);
SNIPE SNIPE(ATSerial);

// 16byte hex key
String lora_app_key = "11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 42";

void setup()
{

    ATSerial.begin(115200);

    // put your setup code here, to run once:
    while (ATSerial.read() >= 0);
    while (!ATSerial);

    loraAndDebugSerialInit();

    DebugSerial.println("SNIPE LoRa Master");

    initCommands();

    /* wifi설정 */
    Serial.begin(115200);
    setupWiFi();
}

void loop()
{
    String diff = newDiffString();   
    Serial.println(diff);  // 이전상태랑 현재상태와 비교해 달라진 점만 json스트링으로 변환
    setStateCurrToPrev();                   // 현재 상태를 이전 상태에 저장
    String response = sendSeatStatus(diff); // http api call
    Serial.println(response);
    resetCommands();                        // 이전 command 삭제
    convertCommands(response);              // http response 받은 json으로 command 생성

    for (int i = 0; i < SEAT_COUNT; i++) // seat loop
    {
        do{
            SNIPE.lora_send(seatCommands[i]);
            String response = SNIPE.lora_recv();
            Serial.println(response);
            if(response == "1" )
            {
                currSeatState[i] = true;
                break;
            }
            else if(response == "0")
            {
                currSeatState[i] = false;
                break;
            }
            delay(500);

        }while(true);
        // TODO 로라로 command 보내고 응답 받기
        // TODO 응답 받아서 seat 상태 currSeatState에 저장
    }
    for (int i = 0; i < DESK_COUNT; i++) // desk loop
    {
        
        do
        {
          SNIPE.lora_send(deskCommands[i]);
          String response = SNIPE.lora_recv();
          if (response == "1") break;
          delay(500);
        } while(true);
        // TODO 로라로 command 보내고 응답 받기
    }
   

    delay(10 * 1000);
}

void loraAndDebugSerialInit()
{
    DebugSerial.begin(115200);
    /* SNIPE LoRa Initialization */
    if (!SNIPE.lora_init())
    {
        DebugSerial.println("SNIPE LoRa Initialization Fail!");
        while (1)
            ;
    }

    /* SNIPE LoRa Set App Key */
    if (!SNIPE.lora_setAppKey(lora_app_key))
    {
        DebugSerial.println("SNIPE LoRa app key value has not been changed");
    }

    /* SNIPE LoRa Set Frequency */
    if (!SNIPE.lora_setFreq(LORA_CH_3))
    {
        DebugSerial.println("SNIPE LoRa Frequency value has not been changed");
    }

    /* SNIPE LoRa Set Spreading Factor */
    if (!SNIPE.lora_setSf(LORA_SF_7))
    {
        DebugSerial.println("SNIPE LoRa Sf value has not been changed");
    }

    /* SNIPE LoRa Set Rx Timeout
     * If you select LORA_SF_12,
     * RX Timout use a value greater than 5000
     */
    if (!SNIPE.lora_setRxtout(5000))
    {
        DebugSerial.println("SNIPE LoRa Rx Timout value has not been changed");
    }
}

void initCommands()
{
    for (int i = 0; i < SEAT_COUNT; i++)
    {
        seatCommands[i] = String("C ") + String(i+1) + String(" 2");
    }
    for (int i = 0; i < DESK_COUNT; i++)
    {
        deskCommands[i] = String("D ") + String(i+1) + String(" 2");
    }
}

void resetCommands()
{
    for (int i = 0; i < SEAT_COUNT; i++)
    {
        seatCommands[i] = String("C ") + String(i+1) + String(" 2");
    }
    for (int i = 0; i < DESK_COUNT; i++)
    {
        deskCommands[i] = String("D ") + String(i+1) + String(" 2");
    }
}

void convertCommands(const String& jsonString)
{
  Serial.println("comvert start");
  Serial.println(jsonString);
    StaticJsonDocument<256> doc;
    deserializeJson(doc, jsonString.c_str());

    JsonArray seatStateChange = doc["seat_state_change"];

    for (JsonObject seatChange : seatStateChange)
    {
        int seatId = seatChange["seat_id"];
        bool isActive = seatChange["is_active"];

        // Convert boolean to integer (1 for True, 0 for False)
        char isActiveChar = isActive ? '1' : '0';

        seatCommands[seatId - 1] = String("C ") + String(seatId) + String(" ") + String(isActiveChar);
    }

    JsonArray seatTimeout = doc["seat_timeout"];

    for (JsonObject seat : seatTimeout)
    {
        int seatId = seat["seat_id"];

        seatCommands[seatId - 1] = String("C ") + String(seatId) + String(" 0");
    }

    JsonArray deskStateChange = doc["desk"];

    for (JsonObject deskChange : deskStateChange)
    {
      Serial.println("deskconv");
        int deskId = deskChange["id"];
        bool isActive = deskChange["state"];

        char isActiveChar = isActive ? '1' : '0';

        deskCommands[deskId - 1] = String("D ") + String(deskId) + String(" ") + String(isActiveChar);
    }

    Serial.println("convert fin");
}

String getDiffStateString()
{
    String prefix = String("{'key':'") + String(SECRET_KEY) + String("','seats':[");
    String suffix = String("]}");
    String nakami = String("");

    for (int i = 0; i < SEAT_COUNT; i++)
    {
        if (prevSeatState[i] == currSeatState[i])
            continue;

        nakami = nakami + String("{'seat_id':") + String(i + 1) + String(",'is_active':") + (currSeatState[i] ? "1" : "0") + String("},");
    }

    return prefix + nakami + suffix;
}

String newDiffString()
{
    int count = 0;
    for (int i = 0; i < SEAT_COUNT; i++)
    {
        if (prevSeatState[i] == currSeatState[i])
            continue;
        count++;
    }
    const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(count) + (count * JSON_OBJECT_SIZE(2));

   DynamicJsonDocument doc(capacity);

   doc["key"] = "verysecret";

  JsonArray seats = doc.createNestedArray("seats");


  // Iterate over the vector of seats and add each seat to the JSON array
  for (int i = 0;i<SEAT_COUNT;i++) {
    JsonObject seat = seats.createNestedObject();
    seat["seat_id"] = i+1;
    seat["is_active"] = prevSeatState[i] == currSeatState[i];
  }

  // Serialize JSON to a string
  String jsonPayload;
  serializeJson(doc, jsonPayload);

  return jsonPayload;
    
}

void setStateCurrToPrev()
{
    for (int i = 0; i < SEAT_COUNT; i++)
    {
        prevSeatState[i] = currSeatState[i];
    }
}

/*
{
  "key": "string",
  "seats": [
    {
      "seat_id": "number",
      "is_active": "bool"
    }
  ]
}
*/