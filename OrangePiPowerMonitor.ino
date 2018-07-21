int status = 0; // 0 - off, 1 - startUp delay(5min), 2 - booting(45s), 3 - working, 4 - powerOf delay(30s), 5 - shutting down(30s), 
bool mainPowerOn = true;
int MAIN_POWER_PIN = 2;
int RELAY_PIN = 3;
int SHUTDOWN_PIN = 4;

unsigned long startUpDelayBegin = 0;
unsigned long startUpDelay = 300000;

unsigned long bootDelayBegin = 0;
unsigned long bootDelay = 45000;

unsigned long powerOffDelayBegin = 0;
unsigned long powerOffDelay = 30000;

unsigned long shutDownDelayBegin = 0;
unsigned long shutDownDelay = 30000;

void setup()
{
	// Initializing serial port for debugging purposes
	Serial.begin(115200);
	pinMode(RELAY_PIN, OUTPUT);
	pinMode(SHUTDOWN_PIN, OUTPUT);
	pinMode(MAIN_POWER_PIN, INPUT);
	Serial.println(status);
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
		status = 4;
	}
	else if (mainPowerOn && status == 4) {
		// Stop powerOffDelay and continue work
		status = 3;
	}
	else if (!mainPowerOn && status == 4) {
		long now = millis();
		if (now - powerOffDelayBegin > powerOffDelay) {
			digitalWrite(SHUTDOWN_PIN, HIGH);
			shutDownDelayBegin = millis();
			status = 5;
		}
	}
	else if (!mainPowerOn && status == 5) {
		long now = millis();
		if (now - shutDownDelayBegin > shutDownDelay) {
			digitalWrite(RELAY_PIN, LOW);
			status = 0;
		}
	}
	delay(1000);
	Serial.println(status);
}
