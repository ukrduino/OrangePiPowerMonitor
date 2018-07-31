#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

SoftwareSerial mySerial(8, 7); // RX, TX
Adafruit_BME280 bme; // I2C - Change sensor address in  I2C, 0x76 or 0x77, in Adafruit_BME280.h (http://playground.arduino.cc/Main/I2cScanner)

long sensorRequestPeriod = 60000; //miliseconds
long lastSensorDataMsg = 0;

int status = 0; // 0 - off, 1 - startUp delay(3min), 2 - booting(30s), 3 - working, 4 - powerOf delay(30s), 5 - shutting down(30s), 
bool mainPowerOn = true;
int MAIN_POWER_PIN = 5;
int RELAY_PIN = 6;
int SHUTDOWN_PIN = 4;

unsigned long startUpDelayBegin = 0;
unsigned long startUpDelay = 180000;

unsigned long bootDelayBegin = 0;
unsigned long bootDelay = 30000;

unsigned long powerOffDelayBegin = 0;
unsigned long powerOffDelay = 30000;

unsigned long shutDownDelayBegin = 0;
unsigned long shutDownDelay = 30000;

void setup()
{
	// Initializing serial port for debugging purposes
	Serial.begin(115200);
	mySerial.begin(115200);
	pinMode(RELAY_PIN, OUTPUT);
	pinMode(SHUTDOWN_PIN, OUTPUT);
	digitalWrite(SHUTDOWN_PIN, LOW);
	pinMode(MAIN_POWER_PIN, INPUT);
	delay(1000);
	if (!bme.begin()) {
		Serial.println("Could not find a valid BME280 sensor, check wiring!");
	}
}

void loop()
{
	mainPowerOn = digitalRead(MAIN_POWER_PIN);
	if (!mainPowerOn && status == 0) {
		// Do nothing wait for mainPowerOn
	}
	else if (mainPowerOn && status == 0) {
		startUpDelayBegin = millis();
		status = 1;
	}
	else if (mainPowerOn && status == 1) {
		long now = millis();
		if (now - startUpDelayBegin > startUpDelay) {
			digitalWrite(RELAY_PIN, HIGH);
			digitalWrite(SHUTDOWN_PIN, LOW);
			bootDelayBegin = millis();
			status = 2;
		}
	}
	else if (!mainPowerOn && status == 1) {
		// Stop bootDelay and wait for mainPowerOn
		status = 0;
	}
	else if (mainPowerOn && status == 2) {
		long now = millis();
		if (now - bootDelayBegin > bootDelay) {
			status = 3;
		}
	}
	else if (!mainPowerOn && status == 3) {
		powerOffDelayBegin = millis();
		mySerial.println("PowerOffDelayBegin");
		status = 4;
	}
	else if (mainPowerOn && status == 4) {
		// Stop powerOffDelay and continue work
		mySerial.println("PowerOffDelayStopped");
		status = 3;
	}
	else if (!mainPowerOn && status == 4) {
		long now = millis();
		if (now - powerOffDelayBegin > powerOffDelay) {
			mySerial.println("ShutDownIn10Seconds");
			delay(10000);
			digitalWrite(SHUTDOWN_PIN, HIGH);
			shutDownDelayBegin = millis();
			status = 5;
		}
	}
	else if (status == 5) {
		long now = millis();
		if (now - shutDownDelayBegin > shutDownDelay) {
			digitalWrite(SHUTDOWN_PIN, LOW);
			digitalWrite(RELAY_PIN, LOW);
			status = 0;
		}
	}
	//delay(1000);
	//Serial.println(status);
	sendTempHumidPressureToOPI();
}

void sendTempHumidPressureToOPI() {
	long now = millis();
	if (now - lastSensorDataMsg > sensorRequestPeriod) {
		lastSensorDataMsg = now;
		float temp = bme.readTemperature();
		delay(1000);
		float pressure = bme.readPressure() / 100.0F;
		delay(1000);
		float humidity = bme.readHumidity();
		Serial.print("BME280|");
		Serial.print(temp);
		Serial.print("|");
		Serial.print(pressure);
		Serial.print("|");
		Serial.print(humidity);
		Serial.println("|");
		mySerial.print("BME280|");
		mySerial.print(temp);
		mySerial.print("|");
		mySerial.print(pressure);
		mySerial.print("|");
		mySerial.print(humidity);
		mySerial.println("|");
	}
}