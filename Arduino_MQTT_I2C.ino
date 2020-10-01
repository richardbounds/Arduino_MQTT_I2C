#include <UIPEthernet.h>
#include <PubSubClient.h>
#include <Wire.h>

#ifndef MQTTHOST
#define MQTTHOST "homeauto"
#define MQTTPORT 1883
#define MQTTCLIENT_ID  "somfy_RTS_1"
#define MQTTUSER "openhab"
#define MQTTPASS "habopen"
#define MQTTTOPIC "somfy_RTS_1"
#endif


// Define Slave I2C Address
#define SLAVE_ADDR 9
 
uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};

EthernetClient ethClient;
PubSubClient mqttClient;

void mqttCallback(char* topic, byte* payload, unsigned int length);
void processI2CCommand(char instruction, int remotePosition);

void setup()
{ 
  Serial.begin(115200);
  if(Ethernet.begin(mac) == 0) {
    Serial.println(F("Ethernet configuration using DHCP failed"));
    for(;;);
  }
  mqttClient.setClient(ethClient);
  mqttClient.setServer(MQTTHOST,MQTTPORT);
  if(mqttClient.connect(MQTTCLIENT_ID, MQTTUSER, MQTTPASS)) {
    Serial.println("MQTT connected");
      mqttClient.subscribe(MQTTTOPIC);
      mqttClient.setCallback(mqttCallback);
    } 
    else 
    {
      Serial.println("MQTT connection failed");
      for(;;);
    } 
  Wire.begin();
}


void loop()
{
  mqttClient.loop();
  Ethernet.maintain();
  delay(10);
}


void mqttCallback(char* topic, byte* payload, unsigned int length) {
  //Serial.println("Message arrived in topic: ");
  //Serial.println(topic);
  char instruction = payload[0];
  int remote = atoi(&payload[1]);
  //Serial.print("Data: " + String(instruction) + "," + String(remote));
  //Serial.println(); 
  processI2CCommand(instruction, remote);
}


void processI2CCommand(char instruction, int remote) {
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(instruction);
  Wire.write(remote);
  Wire.endTransmission();
}
