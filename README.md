# MQTTClient

A simple MQTT client written in C++ using the Paho MQTT C++ library. It reads a configuration file named `mqttClient.ini` and connects to the specified broker. On the first successful connection it publishes a retain message containing the connection time in JSON format to the `STATUS_TOPIC` with QoS 1.

## Build

The helper script `setup.sh` installs the required Paho libraries and builds the
client. Run it on a clean Rocky Linux 9.5 system:

```
./setup.sh
```

After running the script you will have an executable named `mqttClient`.

## Configuration

Sample `mqttClient.ini` (topics follow the three-level pattern `IOT/<DEVICE>/...`):

```
MQTT_SERVER=broker.hivemq.com
MQTT_PORT=1883
TOPIC_LIST=IOT/DEVICE1/DATA,IOT/DEVICE2/DATA
SUBSCRIBE_LIST=IOT/DEVICE1/COMMAND,IOT/DEVICE2/COMMAND
STATUS_TOPIC=IOT/DEVICE1/STATUS
```

## Usage

Run the compiled program in the same directory as the configuration file:

```
./mqttClient
```

The client will automatically reconnect if the connection is lost and will publish periodic demo messages to the topics listed in `TOPIC_LIST`.

