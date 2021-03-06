#include <EmonLib.h>
#include "Arduino.h"
#include <SPI.h>
#include <SD.h>
#include <XBee.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_TMP006.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <BH1750FVI.h>
#include <SHT15.h>
#include <HTU21D.h>  // I2C Timeout modified to allow slower clock rate; no longer produces errors
#include <DHT.h>
#include <DS3231.h>
#include "avr/power.h"
#include "avr/sleep.h"

const char filename[] = "log.csv";
const char headings[] = "Timestamp,Air 1,Air 2,Temp 03,Temp 21,Temp 15,Surface,Humidity 03,Humidity 21,Humidity 15,Light 1,Light 2,LDR,Sound,Current";

// Pin Assignments
#define XBEE_PWR_PIN 5
#define RHT03_PIN 3
#define AIR_TEMP_PIN 9
#define SHT15_CLK_PIN 7
#define SHT15_DATA_PIN 6
#define LDR_PIN 2
#define CHIP_SELECT_PIN 10	// SD Card
#define CURRENT_SENSE_PIN A1
#define MIC_PIN A0

// Temperature
OneWire oneWire(AIR_TEMP_PIN);
DallasTemperature airTempSensors(&oneWire);
Adafruit_TMP006 surfaceTempSensor;

// Humidity
#define RHT03_TYPE DHT22
#define RHT03_COUNT 3
DHT humiditySensor03(RHT03_PIN, RHT03_TYPE, RHT03_COUNT);
HTU21D humiditySensor21;
SHT15 humiditySensor15(SHT15_CLK_PIN, SHT15_DATA_PIN);

// Light
Adafruit_TSL2561_Unified lightSensor = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT);
BH1750FVI lightSensor2;

// Sound
#define MIC_SAMPLE_PERIOD 100

// Power
#define TRANSFORMER_RATIO 2000
#define BURDEN_RESISTOR 220
const float CURRENT_CALIBRATION_FACTOR = TRANSFORMER_RATIO/BURDEN_RESISTOR;
EnergyMonitor currentSensor;

// Misc
DS3231 RTC;
const int INTERRUPT_NUM = 0;
File dataFile;


// Data variables
int airTemp1;
int airTemp2;
int wallTemp;
int caseTemp;

int humidity1;
int humidity2;
int humidity3;

void initialiseTemperatureSensors() 
{
	airTempSensors.begin();
	surfaceTempSensor.begin(TMP006_CFG_1SAMPLE);
}

void initialiseHumiditySensors() 
{
	humiditySensor03.begin();
	humiditySensor21.begin();
}

void initialiseLightSensors() 
{
	lightSensor.begin();
	lightSensor2.begin();
	lightSensor2.SetAddress(Device_Address_H);
	lightSensor2.SetMode(Continuous_H_resolution_Mode);
}






/**
* Initialization stage
* Runs once when power is active
*/
void setup()
{
	// Comms
	Serial.begin(57600);
	Serial.println("Awake");
	
	initialiseDatalog();
	
	Wire.begin();
	RTC.begin();
	
	initialiseTemperatureSensors();
	initialiseHumiditySensors();
	initialiseLightSensors();
	
	
	currentSensor.current(CURRENT_SENSE_PIN, CURRENT_CALIBRATION_FACTOR);
	
	Serial.println(headings);
}

void loop()
{
	// Air temperature
	airTempSensors.requestTemperatures();
	airTemp1 = int(airTempSensors.getTempCByIndex(0)*100);
	float airTemp2 = airTempSensors.getTempCByIndex(1);
	
	// Surface temperature
	float surfaceTemp = surfaceTempSensor.readObjTempC();
	
	// Humidity/Temp
	float temp03 = humiditySensor03.readTemperature();
	float humidity03 = humiditySensor03.readHumidity();
	
	float temp21 = humiditySensor21.readTemperature();
	float humidity21 = humiditySensor21.readHumidity();
	humidity21 = getCorrectHTU21DHumidity(temp21, humidity21);
	
	float temp15 = humiditySensor15.readTemperature();
	float humidity15 = humiditySensor15.readHumidity();
	
	// Luminosity
	sensors_event_t event;
	lightSensor.getEvent(&event);
	int lightLevel = event.light;
	uint16_t lightLevel2 = lightSensor2.GetLightIntensity();
	int ldrLightLevel = analogRead(LDR_PIN);
	
	// Sound pressure level
	int soundLevel = getSoundLevel(MIC_SAMPLE_PERIOD);
	
	//Current
	float currentSense = currentSensor.calcIrms(1000);
	
	// Print values
	DateTime timeStamp = RTC.now();
	//printTimeStamp(timeStamp);
	//Serial.print("  Ta:");
	Serial.print(airTemp1);
	//Serial.print(",");
	//Serial.print(airTemp2);
	//Serial.print("  Th:");
	//Serial.print(temp03);
	//Serial.print(",");
	//Serial.print(temp21);
	//Serial.print(",");
	//Serial.print(temp15);
	//Serial.print("  Ts:");
	//Serial.print(surfaceTemp);
	//Serial.print("  RH:");
	//Serial.print(humidity03);
	//Serial.print(",");
	//Serial.print(humidity21);
	//Serial.print(",");
	//Serial.print(humidity15);
	//Serial.print("  L:");
	//Serial.print(lightLevel);
	//Serial.print(",");
	//Serial.print(lightLevel2);
	//Serial.print(",");
	//Serial.print(ldrLightLevel);
	//Serial.print("  SPL:");
	//Serial.print(soundLevel);
	//Serial.print("  I:");
	//Serial.println(currentSense);
	
	delay(200);
	
	dataFile.print(timeStamp.get());
	//dataFile.print(",");
	//dataFile.print(airTemp1, 2);
	//dataFile.print(",");
	//dataFile.print(airTemp2, 2);
	//dataFile.print(",");
	//dataFile.print(temp03, 2);
	//dataFile.print(",");
	//dataFile.print(temp21, 2);
	//dataFile.print(",");
	//dataFile.print(temp15, 2);
	//dataFile.print(",");
	//dataFile.print(surfaceTemp, 2);
	//dataFile.print(",");
	//dataFile.print(humidity03, 2);
	//dataFile.print(",");
	//dataFile.print(humidity21, 2);
	//dataFile.print(",");
	//dataFile.print(humidity15, 2);
	//dataFile.print(",");
	//dataFile.print(lightLevel);
	//dataFile.print(",");
	//dataFile.print(lightLevel2);
	//dataFile.print(",");
	//dataFile.print(ldrLightLevel);
	//dataFile.print(",");
	//dataFile.print(soundLevel);
	//dataFile.print(",");
	//dataFile.println(currentSense, 2);
	
	dataFile.flush();
	
	// Write to SD card
	delay(200);
}


/**
* Set up the SD card and data file
*/
void initialiseDatalog(){
	pinMode(SS, OUTPUT);
	
	// Initialise SD card
	while(!SD.begin(CHIP_SELECT_PIN)){
		Serial.println("Card failed or not present");
		delay(1000);
	}
	
	Serial.println("Card initialised...");
	
	// Initialse data file
	dataFile = SD.open(filename, O_WRITE |  O_APPEND | O_CREAT);
	if  (!dataFile) {
		Serial.println("error opening file");
		// Wait forever since we cant write data
		while (1) ;
	}
	
	
	dataFile.println(headings);
	
}


/**
* Print the current time in hh:mm:ss
*/
void printTimeStamp(DateTime timeStamp){
	
	Serial.print(timeStamp.date());
	Serial.print(".");
	Serial.print(timeStamp.month());
	Serial.print(".");
	Serial.print(timeStamp.year());
	Serial.print(" ");
	Serial.print(timeStamp.hour(), DEC);
	Serial.print(":");
	Serial.print(timeStamp.minute(), DEC);
	Serial.print(":");
	
	// Add leading 0 if needed
	if(timeStamp.second() < 10){
		Serial.print("0");
	}
	
	Serial.print(timeStamp.second(), DEC);
}


/**
* Get the corrected relative humidity after apply conversion coefficients
*
* @param temperature The temperature of the HTU21D sensor in �C
* @param humidity The relative humidity reading of the HTU21D sensor in %
* @return The corrected humidity value in %
*/
float getCorrectHTU21DHumidity(float temperature, float humidity){
	float correctedHumidity = humidity + (25 - temperature)*-0.15;
	return correctedHumidity;
}


/**
* Get the average sound level over the specified sampling period
*
* @param samplePeriod Listening period for the sampling in ms.
* @return Average sound level in 10-bit counts
*/
int getSoundLevel(int samplePeriod){
	unsigned long startTime = millis();
	long total = 0;
	long count = 0;
	
	while (millis() < (startTime + samplePeriod) && samplePeriod > 0){
		int soundLevel = analogRead(MIC_PIN);
		
		total += soundLevel;
		count += 1;

	}
	
	int average = int(total/count);
	return average;
}

/**
* Put the chip to sleep
* Only a reset, or a button interrupt can wake up the chip
*/
void sleepNow()
{
	// Lowest level sleep - Highest power savings
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	
	sleep_enable();
	
	// Enable button interrupts
	attachInterrupt(INTERRUPT_NUM, clockInterrupt, LOW);
	sleep_mode();
}


void clockInterrupt(){
	
}