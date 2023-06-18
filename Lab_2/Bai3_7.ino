// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>
#include "RTClib.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define EEPROM_SIZE 512
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

RTC_DS1307 rtc;

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 04

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

bool shouldWriteData = true;  // Biến xác định xem liệu nên ghi dữ liệu hay không


/*
 * The setup function. We only start the sensors here
 */
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
    
  // Wire.begin();
  EEPROM.begin(EEPROM_SIZE);
  eraseEEPROM();
  delay(2000); // Pause for 2 seconds

  // Start up the library
  sensors.begin();

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}

/*
 * Main function, get and show the temperature
 */
String bufferStr="";
struct Record {
  float temperature;
  DateTime dateTime;
};

int recordCount = 0;


void loop() {
    start();
    String command = Serial.readStringUntil('\n');
    command.trim();
    Serial.println(command);
    if(command == "") {
    command = bufferStr;
    }
    else {
      if(command == "START" || command =="STOP" ||command =="GETMIN"|| command =="GETMAX" ) {
        bufferStr = command;
      }
    }

    if (bufferStr == "START") {
      shouldWriteData = true;
    } else if (bufferStr == "STOP") {
      shouldWriteData = false;
    } else if (bufferStr == "GETMIN") {
      minTemp();
    } else if (bufferStr == "GETMAX") {
      maxTemp();
    }

  delay(5000); // Delay 5 seconds between temperature readings
}
void start() {
  if(shouldWriteData){
  sensors.requestTemperatures();

  DateTime now = rtc.now();
  float tempC = sensors.getTempCByIndex(0);

  Serial.begin(9600);
  Serial.print("Temp: ");
  Serial.print(tempC);
  Serial.print("|Date: ");
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print("|Time: ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.println(now.second(), DEC);

  saveRecord(tempC, now);
  }
}

void eraseEEPROM() {
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
}

void saveRecord(float temperature, DateTime dateTime) {
  
  if (recordCount < (EEPROM_SIZE / sizeof(Record))) {
    Record record;
    record.temperature = temperature;
    record.dateTime = dateTime;

    int address = recordCount * sizeof(Record);
    Serial.println(String(temperature));

    EEPROM.put(address, record);

    recordCount++;
  }
}

void minTemp() {
  Serial.print("Min Temperature: ");
  float minTemperature = 100.0; // Initialize with a high value
  DateTime minDateTime;

  for (int i = 0; i < recordCount; i++) {
    Record record;
    int address = i * sizeof(Record);
    EEPROM.get(address, record);

    if (record.temperature < minTemperature) {
      minTemperature = record.temperature;
      minDateTime = record.dateTime;
    }
  }

  String message = "Temp:" + String(minTemperature) + "|Date:" + formatDate(minDateTime) + "|Time:" + formatTime(minDateTime);
    Serial.println(message);
}

void maxTemp() {
  Serial.print("Max Temperature: ");
  float maxTemperature = -100.0; // Initialize with a high value
  DateTime minDateTime;
  DateTime maxDateTime;

  for (int i = 0; i < recordCount; i++) {
    Record record;
    int address = i * sizeof(Record);
    EEPROM.get(address, record);

    if (record.temperature > maxTemperature) {
      maxTemperature = record.temperature;
      maxDateTime = record.dateTime;
    }
  }

  String message = "Temp:" + String(maxTemperature) + "|Date:" + formatDate(maxDateTime) + "|Time:" + formatTime(maxDateTime);
    Serial.println(message);

}

String formatDate(DateTime now) {
  return "" + String(now.year()) + "/" + String(now.month()) + "/" + String(now.day());
}

String formatTime(DateTime now) {
  return "" + String(now.hour()) + "/" + String(now.minute()) + "/" + String(now.second());
}