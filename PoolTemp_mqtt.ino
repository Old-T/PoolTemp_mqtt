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
#include <PubSubClient.h>

// Your personal WiFi- and Adafruit IO credentials
// Should define WLAN_SSID, WLAN_PASS and AIO_KEY and AIO_USERNAME
#include "WIFI_and_Adafruit_IO_parameters.h"
//#define test
#define logging
//
// Adafruit IO setup
//

WiFiClient client;
PubSubClient  mqtt("192.168.2.50", 1883, NULL, client);


// Sensor setup
//
#include <OneWire.h>
#include <DallasTemperature.h>
#define OUTDOOR_MEASUREMENT     4
#define POOLTEMP_MEASUREMENT    5
#define OUTDOOR2_MEASUREMENT    6
// Setup for DS18B20
#define ONE_WIRE_BUS 4                // Uses digital pin 3
#define ONE_WIRE_BUS2 5                // Uses digital pin 4
#define ONE_WIRE_BUS3 6   // Uses digital pin 5

OneWire ourWire(ONE_WIRE_BUS);
OneWire ourWire2(ONE_WIRE_BUS2);
OneWire ourWire3(ONE_WIRE_BUS3);
DallasTemperature PoolHouseTemp(&ourWire);
DallasTemperature PoolTemp(&ourWire2);
DallasTemperature OutdoorTemp(&ourWire3);

#define SLEEP_SECONDS 600

int GetTemp( DallasTemperature *TempSensor, float *ReadValue, float MinValid, float MaxValid)
{
    TempSonser->requestTemperatures();
    float value = TempSensor->getTempCByIndex(0);

    if((value > MinValid) && (value < MaxValid))
    {
      *ReadValue = value;
      return 1;
    }
    int LoopVar = 0;

    while( (LoopVar++ <10))
    {
      TempSonser->requestTemperatures();
      value = TempSensor->getTempCByIndex(0);

      if((value > MinValid) && (value < MaxValid))
      {
        *ReadValue = value;
        return 1;
      }
    }
    // No valid value
    return 0;
}

void MQTT_connect()
{
  if (mqtt.connected())
  {
    Serial.println("Already connected");
    return;
  }

  Serial.print("Connecting to MQTT... ");

  int8_t ret;
  while (!mqtt.connect("Pool"))
  {
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("MQTT Connected!");

}

void sendDataToMQTT(float temp, char *MQTT_Key)
{
  Serial.println("Start sending data to Ubuntu MQTT...");
  MQTT_connect();

  char Value[80];
  dtostrf(temp,5,2,Value);
  Serial.println(Value);
  if (mqtt.publish(MQTT_Key, Value))
  {
    Serial.println(temp);
    Serial.println("Sent temperature ok");
  }
  else
    Serial.println(mqtt.state());
    

  mqtt.disconnect();
 
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
  float Temp;
  if( GetTemp( &PoolHouseTemp, &Temp, -30, 70) )
    sendDataToMQTT(Temp, "Home/Temp/Pooltak");
/*
  PoolHouseTemp.requestTemperatures();
  float Temp = PoolHouseTemp.getTempCByIndex(0);
  if (Temp < -30)
  {
    PoolHouseTemp.requestTemperatures();
    Temp = PoolHouseTemp.getTempCByIndex(0);
  }
#ifdef test
  Temp = 22.54;
  sendDataToMQTT(Temp, "Home/Temp/Pooltak");
#else
  Serial.print("PoolHouseTemp ");
  Serial.print(Temp);
  Serial.println(" grader");
  if ( Temp < 70)
    sendDataToMQTT(Temp, "Home/Temp/Pooltak");
#endif
*/

  if( GetTemp( &PoolTemp, &Temp, -10, 50) )
    sendDataToMQTT(Temp, "Home/Temp/Pool");

/*
  PoolTemp.requestTemperatures();
  Temp = PoolTemp.getTempCByIndex(0);
  if (Temp < -5)
  {
    PoolTemp.requestTemperatures();
    Temp = PoolHouseTemp.getTempCByIndex(0);
  }
#ifdef test
  Temp = 21.52;
  sendDataToMQTT(Temp, "Home/Temp/Pool");
#else
  Serial.print("PoolTemp ");
  Serial.print(Temp);
  Serial.println(" grader");
  if ( Temp < 70)
    sendDataToMQTT(Temp, "Home/Temp/Pool");
#endif 
*/
  if( GetTemp( &OutdoorTemp, &Temp, -40, 40) )
    sendDataToMQTT(Temp, "Home/Temp/Utetemp");

/*
  OutdoorTemp.requestTemperatures();
  Temp = OutdoorTemp.getTempCByIndex(0);
  if (Temp < -100)
  {
    OutdoorTemp.requestTemperatures();
    Temp = PoolHouseTemp.getTempCByIndex(0);
  }
#ifdef test
  Temp = 21.52;
  sendDataToMQTT(Temp, "Home/Temp/Utetemp");
#else
  Serial.print("Utetemp ");
  Serial.print(Temp);
  Serial.println(" grader");
  if ( Temp < 70)
    sendDataToMQTT(Temp, "Home/Temp/Utetemp");
#endif 
*/

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

