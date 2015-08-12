#include <SPI.h>
#include <MySensor.h>
#include <DHT.h>

#define NODE_ID 10
#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define HUMIDITY_SENSOR_DIGITAL_PIN 3
#define BATTERY_SENSOR_PIN A0

unsigned long SLEEP_TIME = 60000; // Sleep time between reads (in milliseconds)

MySensor gw;
DHT dht;

float lastTemp;
float lastHum;
int oldBatteryPcnt = 0;
int minutes = 0;

boolean metric = true;
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);

void setup()
{
  gw.begin(NULL, NODE_ID, false);
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN);

  // Prepare for the battery level setup
  analogReference(INTERNAL);

  // Send the Sketch Version Information to the Gateway
  gw.sendSketchInfo("Humidity/Temperature/Battery", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);

  metric = gw.getConfig().isMetric;
}

void sendBatteryLevel() {
  if(minutes % 15 == 0) {
    minutes = 0;
    int sensorValue = analogRead(BATTERY_SENSOR_PIN);
    int batteryPcnt = sensorValue / 10;

    if (oldBatteryPcnt != batteryPcnt) {
      // Power up radio after sleep
      gw.sendBatteryLevel(batteryPcnt);
      oldBatteryPcnt = batteryPcnt;
    }
  }
  else {
    minutes++;
  }
}

void sendTemperature() {
  float temperature = dht.getTemperature();
  if (temperature != lastTemp) {
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    gw.send(msgTemp.set(temperature, 1));
  }
}

void sendHumidity() {
  float humidity = dht.getHumidity();
  if (humidity != lastHum) {
    lastHum = humidity;
    gw.send(msgHum.set(humidity, 1));
  }
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  sendTemperature();
  sendHumidity();
  // sendBatteryLevel();

  gw.sleep(SLEEP_TIME); //sleep a bit
}

