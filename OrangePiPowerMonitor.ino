int status = 0; // 0 - off, 1 - startUp delay(3min), 2 - booting(30s), 3 - working, 4 - powerOf delay(30s), 5 - shutting down(30s), 
bool mainPowerOn = true;
int MAIN_POWER_PIN = 5;
int RELAY_PIN = 6;
bool RELAY_ENABLED = false;
int SHUTDOWN_PIN = 9;
int HARD_RESET_PIN = 4;

unsigned long startUpDelayStart = 0;
unsigned long startUpDelay = 180000;

unsigned long bootDelayStart = 0;
unsigned long bootDelay = 30000;

unsigned long powerOffDelayStart = 0;
unsigned long powerOffDelay = 30000;

unsigned long shutDownDelayStart = 0;
unsigned long shutDownDelay = 30000;

void setup()
{
	// Initializing serial port for debugging purposes
	Serial.begin(115200);
	pinMode(RELAY_PIN, OUTPUT);
	pinMode(SHUTDOWN_PIN, OUTPUT);
	digitalWrite(SHUTDOWN_PIN, LOW);
	pinMode(MAIN_POWER_PIN, INPUT);
	pinMode(HARD_RESET_PIN, INPUT);
	digitalWrite(HARD_RESET_PIN, HIGH);
	delay(100);
}

void loop()
{
	mainPowerOn = digitalRead(MAIN_POWER_PIN);
	if (!mainPowerOn && status == 0) {
		// Do nothing wait for mainPowerOn
	}
	else if (mainPowerOn && status == 0) {
		startUpDelayStart = millis();
		status = 1;
	}
	else if (mainPowerOn && status == 1) {
		//Wait for charge of sypercapacitors
		long now = millis();
		if (now - startUpDelayStart > startUpDelay) {
			//Start Raspberry
			digitalWrite(RELAY_PIN, HIGH);
			RELAY_ENABLED = true;
			digitalWrite(SHUTDOWN_PIN, LOW);
			bootDelayStart = millis();
			status = 2;
		}
	}
	else if (!mainPowerOn && status == 1) {
		// Stop startUpDelay and wait for mainPowerOn
		status = 0;
	}
	else if (mainPowerOn && status == 2) {
		long now = millis();
		if (now - bootDelayStart > bootDelay) {
			// Raspberry boot finished
			status = 3;
		}
	}
	else if (!mainPowerOn && status == 3) {
		powerOffDelayStart = millis(); //wait for short power interrupts 30s
		status = 4;
	}
	else if (mainPowerOn && status == 4) {
		// Stop powerOffDelay and continue work
		status = 3;
	}
	else if (!mainPowerOn && status == 4) {
		long now = millis();
		if (now - powerOffDelayStart > powerOffDelay) {
			//Shutdown signal sent to Raspberri
			digitalWrite(SHUTDOWN_PIN, HIGH);
			shutDownDelayStart = millis();
			status = 5;
		}
	}
	else if (status == 5) {
		long now = millis();
		if (now - shutDownDelayStart > shutDownDelay) {
			//Power disconnected
			digitalWrite(SHUTDOWN_PIN, LOW);
			digitalWrite(RELAY_PIN, LOW);
			RELAY_ENABLED = false;
			status = 0;
		}
	}
	else if (digitalRead(HARD_RESET_PIN) == LOW && RELAY_ENABLED) {
		delay(300);
		digitalWrite(RELAY_PIN, LOW);
		delay(2000);
		digitalWrite(RELAY_PIN, HIGH);		
	}
	delay(1000);
	Serial.print(millis() / 1000);
	Serial.print(" power: ");
	Serial.print(mainPowerOn);
	Serial.print(", status: ");
	Serial.println(status);
}