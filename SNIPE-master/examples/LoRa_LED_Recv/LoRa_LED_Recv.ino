#include <SoftwareSerial.h>
#include "SNIPE.h"

#define TXpin 11
#define RXpin 10
#define ATSerial Serial

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
      
  DebugSerial.println("SNIPE LoRa LED Recv");

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
        String ver = SNIPE.lora_recv();
        
        if (ver == "123" )
        {
            digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
            DebugSerial.println("SNIPE LoRa LED on");
            DebugSerial.println(SNIPE.lora_getRssi());
            DebugSerial.println(SNIPE.lora_getSnr());             
            SNIPE.lora_send("Ack");
        }
        else if (ver == "456")
        {
            digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
            DebugSerial.println("SNIPE LoRa LED off");
            DebugSerial.println(SNIPE.lora_getRssi());
            DebugSerial.println(SNIPE.lora_getSnr());             
            SNIPE.lora_send("Ack");      
        }
        else
        {
            DebugSerial.println("recv Error!!!");
            delay(1000);
        }
        delay(1000);
}
