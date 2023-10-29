/**
 * Author: Joshua Dotto
 * Date: 2023-10-23
 * 
 * Firmware that runs on an IoT end-device.
 * 
 * Code tracks night activity based on the 
 * state of the room lights (on/off) using
 * a photo-resistor and a real-time clock module.
 * The temperature of the room is also recorded
 * every five minutes.
 */

#include <Arduino.h>
#include <RTClib.h> // real-time clock
#include <Adafruit_SHT4x.h> // temperature sensor

#define LIGHT_LEVEL_THRESHOLD 40.0 // lights off < 40.0% > lights on (determined from tests)
#define CHECK_LIGHTS_INTERVAL 1000UL // 1000 milliseconds (1 second)
#define CHECK_TEMPERATURE_INTERVAL 5000UL // 300000 milliseconds (5 minutes)

const char START[] = "19:27:00"; // start tracking night activity at 6 PM
const char END[] = "19:28:00"; // stop tracking night activity at 6 AM

char timestamp[20];  // "yyyy-mm-dd hh:mm:ss" plus null-terminator
char lights_payload[42]; // "{"lights":1,"time":"yyyy-mm-dd hh:mm:ss"}" plus null-terminator
char temperature_payload[44]; // "{"temp":20.13,"time":"yyyy-mm-dd hh:mm:ss"}" plus null-terminator
char temperature_string[6]; // sprintf() does not directly support floats on arduino. See https://www.programmingelectronics.com/sprintf-arduino/

RTC_DS3231 rtc;
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

//======= Helper functions =======

/**
 * returns timestamp as "yyyy-mm-dd hh:mm:ss" for events
 */

char* getTimestamp() {

  DateTime now = rtc.now();
  sprintf(timestamp, "%d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  return timestamp;
}

/**
 * returns time of the day ("hh:mm:ss") to
 * determine the monitoring window
 */ 

char* getTimeOnly(char* timestamp){
  return strchr(timestamp, ' ') + 1;
}

/**
 * returns light level percentage
 * 100% ==> lights on and sensor not in shadow
 * 0% ==> lights off
 */ 

float getLightLevel(){
  float lit_room_sensor_voltage = 4.5; // obtained from multiple tests
  float sensor_voltage = 5 * analogRead(A0) / 1023.0; //reading photo-resistor with arduino 10-bit ADC (5V logic)
  return 100 * sensor_voltage / lit_room_sensor_voltage;
}

//======= End helper functions =======

void setup() {

  Serial.begin(115200);

  if (!rtc.begin()) {
    Serial.println("RTC failed");
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // reset RTC with compile timestamp
  }

  if (!sht4.begin()) {
    Serial.println("Temp sensor failed");
  }
}

void loop() {
  
  char* current_date_time = getTimestamp(); // yyyy-mm-dd hh:mm:ss
  char* current_time = getTimeOnly(current_date_time); // hh:mm:ss
  unsigned long time_elapsed = millis(); // time elapsed (milliseconds) since program start

  static unsigned long last_temperature_check = 0UL;
  static unsigned long last_lights_check = 0UL;
  static bool room_is_lit = false;
  static bool can_monitor = true;

  if(strcmp(current_time, START) == 0){
    can_monitor = true;
  }

  if(strcmp(current_time, END) == 0){
    can_monitor = false;
  }

  //Serial.println(start_monitoring);
  //Serial.println(current_time);

  if((time_elapsed - last_lights_check >= CHECK_LIGHTS_INTERVAL) && can_monitor){

    if(getLightLevel() > LIGHT_LEVEL_THRESHOLD && !room_is_lit){

      sprintf(lights_payload, "{\"lights\":%d,\"time\":\"%s\"}", 1, current_date_time); // 1 for on (consistent string length)
      Serial.println(lights_payload);
      room_is_lit = true;
    }

    if(getLightLevel() < LIGHT_LEVEL_THRESHOLD && room_is_lit){

      sprintf(lights_payload, "{\"lights\":%d,\"time\":\"%s\"}", 0, current_date_time); // 0 for off (consistent string length)
      Serial.println(lights_payload);
      room_is_lit = false;
    }

    last_lights_check = time_elapsed; // update last lights check
  }

  if((time_elapsed - last_temperature_check >= CHECK_TEMPERATURE_INTERVAL) && can_monitor){

    sensors_event_t humidity, temp;
    sht4.getEvent(&humidity, &temp);
    // sprintf() does not directly support floats on arduino. See https://www.programmingelectronics.com/sprintf-arduino/
    dtostrf(temp.temperature, 2, 2, temperature_string);
    sprintf(temperature_payload, "{\"temp\":%s,\"time\":\"%s\"}", temperature_string, current_date_time);
    Serial.println(temperature_payload);

    last_temperature_check = time_elapsed; // update last temperature check
  }   
}
