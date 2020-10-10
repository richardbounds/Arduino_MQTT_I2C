#include <UIPEthernet.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "config.h"

// Define Slave I2C Address
#define SLAVE_ADDR 9

#define REMOTE_COUNT 32

#ifndef MQTTHOST
#define MQTTHOST "openhab"
#define MQTTPORT 1883
#define MQTTCLIENT_ID  "somfy_RTS_1"
#define MQTTUSER "mqttuser"
#define MQTTPASS "mqttpass"
#define MQTTTOPIC "somfy_RTS_1/remote/%d" //MQTT topics of the form somfy_RTS_1/remote/{0,1,2,3...31}
#endif


#define BLUE_LIGHT_PIN 4
#define GREEN_LIGHT_PIN 3
#define RED_LIGHT_PIN 2
 
uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
char* mqtt_topic = MQTTTOPIC;

long lastReconnectAttempt = 0;
long mqttReconnectTimeout = 5000;

EthernetClient ethClient;
PubSubClient mqttClient;

void mqttCallback(char* topic, byte* payload, unsigned int length);
void processI2CCommand(char instruction, int remotePosition);

void setup()
{ 

  pinMode(RED_LIGHT_PIN, OUTPUT);
  pinMode(GREEN_LIGHT_PIN, OUTPUT);
  pinMode(BLUE_LIGHT_PIN, OUTPUT);
  RGB_color(255, 0, 0); // Red at startup

  Serial.begin(9600);
  Serial.println(F("=====Starting======"));
  if(Ethernet.begin(mac)) {
    Serial.println(Ethernet.localIP());
  }
  mqttClient.setClient(ethClient);
  mqttClient.setServer(MQTTHOST,MQTTPORT);
  mqttClient.setCallback(mqttCallback);
  Wire.begin();
}


void loop()
{
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Link status: Off");
    RGB_color(255, 0, 0); // Red
    delay(1000);
  }
  int etherStatus = Ethernet.maintain();
  if (etherStatus == 1) {
    Serial.println(F("Renew failed"));
    RGB_color(255, 0, 0); // Red
    delay(1000);
  } else if (etherStatus == 3) {
    Serial.println(F("Rebind failed"));
    RGB_color(255, 0, 0); // Red
    delay(1000);
  } else {
    if (!mqttClient.connected()) {
      RGB_color(255, 255, 0); // Yellow
      long now = millis();
      // Attempt to reconnect not more than every 5 secs
      if (now - lastReconnectAttempt > 5000) {     
        lastReconnectAttempt = now;
        Serial.println(F("trying to connect to mqtt..."));
        if (connectMqtt()) {
          lastReconnectAttempt = 0;
          Serial.println(F("connect to mqtt OK."));
        }
        else
        {
          Serial.println(F("connect to mqtt FAIL."));
        }
      }
    } else {
      RGB_color(0, 255, 0); // Green
      // Client connected
      mqttClient.loop();
    }
  }
  delay(10);
}

boolean connectMqtt() {
  if(mqttClient.connect(MQTTCLIENT_ID, MQTTUSER, MQTTPASS)) {
      for (int i=0; i< REMOTE_COUNT; i++) {
        char topic [strlen(MQTTTOPIC)]; //%d gives us 2 chars for the remote
        sprintf(topic, MQTTTOPIC, i);
        Serial.println(topic);        
        mqttClient.subscribe(topic);
      }
  } 
  return mqttClient.connected();
}

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
{
  analogWrite(RED_LIGHT_PIN, red_light_value);
  analogWrite(GREEN_LIGHT_PIN, green_light_value);
  analogWrite(BLUE_LIGHT_PIN, blue_light_value);
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
  char instruction = tolower(payload[0]); //support [u]p,[d]own,[s]top
  Serial.print(F("Remote: "));
  Serial.print(remote);
  Serial.print(F(", instruction: "));
  Serial.println(instruction); 
  processI2CCommand(instruction, remote);
}


void processI2CCommand(char instruction, int remote) {
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(instruction);
  Wire.write(remote);
  Wire.endTransmission();
}
