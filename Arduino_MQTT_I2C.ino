#include <UIPEthernet.h>
#include <PubSubClient.h>
#include <Wire.h>


// Define Slave I2C Address
#define SLAVE_ADDR 9

#define REMOTE_COUNT 32

#ifndef MQTTHOST
#define MQTTHOST "homeauto"
#define MQTTPORT 1883
#define MQTTCLIENT_ID  "somfy_RTS_1"
#define MQTTUSER "openhab"
#define MQTTPASS "habopen"
#define MQTTTOPIC "somfy_RTS_1/remote/%d" //MQTT topics of the form somfy_RTS_1/remote/{0,1,2,3...31}
#endif


 
uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
char* mqtt_topic = MQTTTOPIC;

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
      for (int i=0; i< REMOTE_COUNT; i++) {
        char topic [strlen(MQTTTOPIC)]; //%d gives us 2 chars for the remote
        sprintf(topic, MQTTTOPIC, i);
        Serial.println(topic);        
        mqttClient.subscribe(topic);
      }
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
  Serial.println(topic);
  //remote id should be the last token in the topic split by '/'
  char *remote_str;
  int remote =-1;
  remote_str = strtok(topic, "/");
  while( remote_str != NULL ) {
     remote = atoi(remote_str);
     remote_str = strtok(NULL, "/");
  }
  char instruction = payload[0]; //support [u]p,[d]own,[s]top
  Serial.print("Remote: ");
  Serial.print(remote);
  Serial.print(", instruction: ");
  Serial.println(instruction); 
  processI2CCommand(instruction, remote);
}


void processI2CCommand(char instruction, int remote) {
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(instruction);
  Wire.write(remote);
  Wire.endTransmission();
}
