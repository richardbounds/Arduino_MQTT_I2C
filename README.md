# Arduino_MQTT_I2C

Arduino nano ethernet mqtt->I2C bridge to drive [Arduino_SomfyRTS](https://github.com/richardbounds/Arduino_SomfyRTS)

- Arduino Nano v3 (clone)
- ENC28J60 Ethernet Shield 
- RGB status LED connected on pins D2,D3,D4
- I2C on pins A4, A5

Mine is powered by a 5v PoE splitter, connected to a [Mosquitto](https://mosquitto.org/) broker for use with [openHAB](https://www.openhab.org/addons/bindings/mqtt/).