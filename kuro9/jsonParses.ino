#include <ArduinoJson.h>

#define MAX_COM 20

#define SEAT_COUNT 3
#define DESK_COUNT 1

String seatCommands[SEAT_COUNT];
String deskCommands[DESK_COUNT];

String jsonResponse;

void setup()
{
    Serial.begin(9600);
    jsonResponse = R"(
    {
      "seat_state_change": [
        {"seat_id": 1, "is_active": true},
        {"seat_id": 2, "is_active": false},
        {"seat_id": 3, "is_active": true}
      ],
      "seat_timeout": [
        {"seat_id": 1, "description": "string"}
      ],
      "desk": [
        {"id": 1, "state": true}
      ]
    }
  )";
}

void loop()
{
    initCommands();

    convertCommands(jsonResponse);

    for (String &command : seatCommands)
    {
        Serial.println(command);
    }
    for (String &command : deskCommands)
    {
        Serial.println(command);
    }

    while (1)
    {
    };
}

void initCommands()
{
    for (int i = 1; i <= SEAT_COUNT; i++)
    {
        seatCommands[i] = String("C ") + String(i) + String(" 2");
    }
    for (int i = 1; i <= SEAT_COUNT; i++)
    {
        deskCommands[i] = String("D ") + String(i) + String(" 2")
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
        int isActiveInt = isActive ? 1 : 0;

        seatCommands[seatId - 1] = "C " + String(seatId) + " " + String(isActiveInt);
    }

    JsonArray seatTimeout = doc["seat_timeout"];

    for (JsonObject seat : seatTimeout)
    {
        int seatId = seat["seat_id"];

        seatCommands[seatId - 1] = "C " + String(seatId) + " 0";
    }

    JsonArray deskStateChange = doc["desk"];

    for (JsonObject deskChange : deskStateChange)
    {
        int deskId = deskChange["id"];
        bool isActive = deskChange["state"];

        deskCommands[deskId - 1] = "D " + String(deskId) + " " + String(isActive ? 1 : 0);
    }
}
