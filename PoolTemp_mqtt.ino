//
// This sketch uses an ESP8266-based Adafruit Feather Huzzah for measuring
// temperature and humidity with a DHT11 sensor
// The values are sent as feeds to an Adafruit IO account via Adafruit MQTT publish calls
//
// To save power, deep sleep from the ESP8266 library is used
// To wake up the board a signal on pin 16 is sent to the reset input
// (so you need a wire between D16 and the reset pin)
// Note! The connection D16 -> Reset has to be removed when uploading the sketch!
// All activity happens in the setup() method as the board is reset after sleeping
//

#include <ESP8266WiFi.h>
//#include "Adafruit_MQTT.h"
//#include "Adafruit_MQTT_Client.h"
#include <PubSubClient.h>

// Your personal WiFi- and Adafruit IO credentials
// Should define WLAN_SSID, WLAN_PASS and AIO_KEY and AIO_USERNAME
#include "WIFI_and_Adafruit_IO_parameters.h"

//
// Adafruit IO setup
//

WiFiClient client;
PubSubClient  mqtt("192.168.2.50", 1883, client);


// Sensor setup
//
#include <OneWire.h>
#include <DallasTemperature.h>
#define OUTDOOR_MEASUREMENT     4
#define POOLTEMP_MEASUREMENT    5
// Setup for DS18B20
#define ONE_WIRE_BUS 4                // Uses digital pin 3
#define ONE_WIRE_BUS2 5                // Uses digital pin 4
OneWire ourWire(ONE_WIRE_BUS);
OneWire ourWire2(ONE_WIRE_BUS2);
DallasTemperature PoolHouseTemp(&ourWire);
DallasTemperature PoolTemp(&ourWire2);

#define SLEEP_SECONDS 600

void MQTT_connect()
{
  mqtt.setServer('192.168.2.50', 1883 );
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  int8_t ret;
  while ((ret = mqtt.connect("Pool")) != 0)
  {
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("MQTT Connected!");

}

void sendDataToAdafruitIO(float pooltemp, float poolhousetemp)
{
  Serial.println("Start sending data to Ubuntu MQTT...");

  MQTT_connect();
  char Value[80];
  sprintf( Value, "%d", pooltemp);
  if (mqtt.publish("Home/Temp/Pool", Value))
  {
    Serial.println(pooltemp);
    Serial.println("Sent temperature ok");
  }

  sprintf( Value, "%d", poolhousetemp);
  if (mqtt.publish("Home/Temp/Pool", Value))
  {
    Serial.println(poolhousetemp);
    Serial.println("Sent poolhouse temp ok");
  }
  
  unsigned long endTime = millis();
}

void setupWiFi()
{
  unsigned long startTime = millis();

  // Setup serial port access.
  Serial.begin(115200);

  // Connect to WiFi access point.
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  //Serial.println();

  //Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  unsigned long endTime = millis();
}

void setup()
{
  // Serial.println("Start!");
  setupWiFi();

  PoolHouseTemp.requestTemperatures();
  float poolHouseTemp = PoolHouseTemp.getTempCByIndex(0);
  if (poolHouseTemp < 0)
  {
    PoolHouseTemp.requestTemperatures();
    poolHouseTemp = PoolHouseTemp.getTempCByIndex(0);
  }

  Serial.print("PoolHouseTemp ");
  Serial.print(poolHouseTemp);
  Serial.println(" grader");

  PoolTemp.requestTemperatures();
  float poolTemp = PoolTemp.getTempCByIndex(0);
  if (poolTemp < 0)
  {
    PoolTemp.requestTemperatures();
    poolTemp = PoolHouseTemp.getTempCByIndex(0);
  }
  Serial.print("PoolTemp ");
  Serial.print(poolTemp);
  Serial.println(" grader");

  if ( (poolHouseTemp < 70) && (poolTemp < 70))
    sendDataToAdafruitIO(poolTemp, poolHouseTemp);

  // Put the board to deep sleep to save power. Will send a signal on D16 when it is time to wake up.
  // Thus, connect D16 to the reset pin. After reset, setup will be executed again.
  Serial.print("Going to deep sleep for ");
  Serial.print(SLEEP_SECONDS);
  Serial.println(" seconds");
  ESP.deepSleep(SLEEP_SECONDS * 1000000);
}

void loop()
{
  // nothing to do here as setup is called when waking up after deep sleep
}

