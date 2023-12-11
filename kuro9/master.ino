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

bool prevSeatState[SEAT_COUNT];
bool currSeatState[SEAT_COUNT];

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
    while (ATSerial.read() >= 0)
        ;
    while (!ATSerial)
        ;

    loraAndDebugSerialInit();

    DebugSerial.println("SNIPE LoRa Master");

    prevSeatState = {false};
    currSeatState = {false};
    initCommands();

    /* wifi설정 */
    Serial.begin(115200);
    setupWiFi();
}

void loop()
{
    String diff = getDiffStateString();     // 이전상태랑 현재상태와 비교해 달라진 점만 json스트링으로 변환
    setStateCurrToPrev();                   // 현재 상태를 이전 상태에 저장
    String response = sendSeatStatus(diff); // http api call
    resetCommands();                        // 이전 command 삭제
    convertCommands(response);              // http response 받은 json으로 command 생성

    for (int i = 0; i < SEAT_COUNT; i++) // seat loop
    {
        // TODO 로라로 command 보내고 응답 받기
        // TODO 응답 받아서 seat 상태 currSeatState에 저장
    }
    for (int i = 0; i < DESK_COUNT; i++) // desk loop
    {
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
    if (!SNIPE.lora_setFreq(LORA_CH_8))
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
        seatCommands[i] = String("C ") + String(i) + String(" 2");
    }
    for (int i = 0; i < DESK_COUNT; i++)
    {
        deskCommands[i] = String("D ") + String(i) + String(" 2")
    }
}

void resetCommands()
{
    for (int i = 0; i < SEAT_COUNT; i++)
    {
        seatCommands[i].setCharAt(4, '2');
    }
    for (int i = 0; i < DESK_COUNT; i++)
    {
        deskCommands[i].setCharAt(4, '2');
    }
}

void convertCommands(const String &jsonString)
{
    StaticJsonDocument<256> doc;
    deserializeJson(doc, jsonString.c_str());

    JsonArray seatStateChange = doc["seat_state_change"];

    for (JsonObject seatChange : seatStateChange)
    {
        int seatId = seatChange["seat_id"];
        bool isActive = seatChange["is_active"];

        // Convert boolean to integer (1 for True, 0 for False)
        char isActiveChar = isActive ? '1' : '0';

        seatCommands[seatId - 1].setCharAt(4, isActiveChar);
    }

    JsonArray seatTimeout = doc["seat_timeout"];

    for (JsonObject seat : seatTimeout)
    {
        int seatId = seat["seat_id"];

        seatCommands[seatId - 1].setCharAt(4, '0');
    }

    JsonArray deskStateChange = doc["desk"];

    for (JsonObject deskChange : deskStateChange)
    {
        int deskId = deskChange["id"];
        bool isActive = deskChange["state"];

        char isActiveChar = isActive ? '1' : '0';

        deskCommands[deskId - 1].setCharAt(4, isActiveChar);
    }
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
