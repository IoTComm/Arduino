/*






slave (책상)

릴레이만 조작





*/
#include <SoftwareSerial.h>
#include "SNIPE.h"

#define LED 6
#define ID 2

#define TXpin 11
#define RXpin 10
#define ATSerial Serial

// 16byte hex key
String lora_app_key = "11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 00";

SoftwareSerial DebugSerial(RXpin, TXpin);
SNIPE SNIPE(ATSerial);

void setup()
{
    ATSerial.begin(115200);

    pinMode(LED, OUTPUT);

    // put your setup code here, to run once:
    while (ATSerial.read() >= 0)
    {
    }
    while (!ATSerial)
        ;

    DebugSerial.begin(115200);

    /* SNIPE LoRa Initialization */
    if (!SNIPE.lora_init())
    {
        DebugSerial.println("SNIPE LoRa Initialization Fail!");
        while (1)
            ;
    }

    /* SNIPE LoRa Set Appkey */
    if (!SNIPE.lora_setAppKey(lora_app_key))
    {
        DebugSerial.println("SNIPE LoRa app key value has not been changed");
    }

    /* SNIPE LoRa Set Frequency */
    if (!SNIPE.lora_setFreq(LORA_CH_1))
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

    DebugSerial.println("SNIPE LoRa PingPong Test");
}

void loop()
{
    String ver = SNIPE.lora_recv();
    delay(300);

    DebugSerial.println(ver);
    char idChar = ID + '0';

    if (ver[0] == idChar)
    {
        DebugSerial.println("recv success");
        DebugSerial.println(SNIPE.lora_getRssi());
        DebugSerial.println(SNIPE.lora_getSnr());

        if (ver[2] == '1')
        {
            digitalWrite(LED, HIGH);
            else digitalWrite(LED, LOW);

            String ack = String(ID) + String(" ACK") if (SNIPE.lora_send(ID))
            {
                DebugSerial.println("send ACK success");
            }
            else
            {
                DebugSerial.println("send fail");
                delay(500);
            }
        }
        delay(1000);
    }
}
