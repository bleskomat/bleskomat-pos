#include "main.h"

uint16_t amountCentsDivisor = 1;
unsigned short maxNumKeysPressed = 12;
unsigned int sleepModeDelay;

void setup() {
	Serial.begin(MONITOR_SPEED);
	spiffs::init();
	config::init();
	logger::init();
	logger::write(firmwareName + ": Firmware version = " + firmwareVersion + ", commit hash = " + firmwareCommitHash);
	logger::write(config::getConfigurationsAsString());
	power::init();
	jsonRpc::init();
	screen::init();
	keypad::init();
	const unsigned short fiatPrecision = config::getUnsignedShort("fiatPrecision");
	amountCentsDivisor = std::pow(10, fiatPrecision);
	if (fiatPrecision > 0) {
		maxNumKeysPressed--;
	}
	sleepModeDelay = config::getUnsignedInt("sleepModeDelay");
}

std::string leftTrimZeros(const std::string &keys) {
	return std::string(keys).erase(0, std::min(keys.find_first_not_of('0'), keys.size() - 1));
}

double keysToAmount(const std::string &t_keys) {
	if (t_keys == "") {
		return 0;
	}
	const std::string trimmed = leftTrimZeros(t_keys);
	double amount = std::stod(trimmed.c_str());
	if (amountCentsDivisor > 1) {
		amount = amount / amountCentsDivisor;
	}
	return amount;
}

std::string pin;
std::string keysBuffer = "";

unsigned long lastActivityTime = millis();
bool isFakeSleeping = false;

void runAppLoop() {
	if (sleepModeDelay > 0 && millis() - lastActivityTime > sleepModeDelay) {
		if (power::isUSBPowered()) {
			// The battery does not charge while in deep sleep mode.
			// So let's just turn off the screen instead.
			screen::sleep();
			isFakeSleeping = true;
		} else {
			power::sleep();
		}
	} else if (isFakeSleeping) {
		screen::wakeup();
		isFakeSleeping = false;
	}
	keypad::loop();
	const std::string currentScreen = screen::getCurrentScreen();
	if (currentScreen == "") {
		screen::showHomeScreen();
	}
	const std::string keyPressed = keypad::getPressedKey();
	if (keyPressed != "") {
		logger::write("Key pressed: " + keyPressed);
		lastActivityTime = millis();
	}
	if (currentScreen == "home") {
		if (keyPressed == "") {
			// Do nothing.
		} else if (keyPressed == "*") {
			keysBuffer = "";
			screen::showEnterAmountScreen(keysToAmount(keysBuffer));
		} else if (keyPressed == "#") {
			screen::showEnterAmountScreen(keysToAmount(keysBuffer));
		} else {
			if (keyPressed != "0" || keysBuffer != "") {
				keysBuffer += keyPressed;
			}
			screen::showEnterAmountScreen(keysToAmount(keysBuffer));
		}
	} else if (currentScreen == "enterAmount") {
		if (keyPressed == "") {
			// Do nothing.
		} else if (keyPressed == "*") {
			keysBuffer = "";
			screen::showEnterAmountScreen(keysToAmount(keysBuffer));
		} else if (keyPressed == "#") {
			const double amount = keysToAmount(keysBuffer);
			if (amount > 0) {
				pin = util::generateRandomPin();
				const std::string signedUrl = util::createLnurlPay(amount, pin);
				const std::string encoded = util::lnurlEncode(signedUrl);
				std::string qrcodeData = "";
				// Allows upper or lower case URI schema prefix via a configuration option.
				// Some wallet apps might not support uppercase URI prefixes.
				qrcodeData += config::getString("uriSchemaPrefix");
				// QR codes with only uppercase letters are less complex (easier to scan).
				qrcodeData += util::toUpperCase(encoded);
				screen::showPaymentQRCodeScreen(qrcodeData);
				logger::write("Payment request shown: \n" + signedUrl);
			}
		} else if (keysBuffer.size() < maxNumKeysPressed) {
			if (keyPressed != "0" || keysBuffer != "") {
				keysBuffer += keyPressed;
			}
			logger::write("keysBuffer = " + keysBuffer);
			screen::showEnterAmountScreen(keysToAmount(keysBuffer));
		}
	} else if (currentScreen == "paymentQRCode") {
		if (keyPressed == "#") {
			screen::showPaymentPinScreen(pin);
		} else if (keyPressed == "1") {
			screen::adjustContrast(-10);// decrease contrast
		} else if (keyPressed == "4") {
			screen::adjustContrast(10);// increase contrast
		}
	} else if (currentScreen == "paymentPin") {
		if (keyPressed == "*") {
			keysBuffer = "";
			screen::showHomeScreen();
		}
	}
}

void loop() {
	logger::loop();
	jsonRpc::loop();
	if (!jsonRpc::hasPinConflict() || !jsonRpc::inUse()) {
		runAppLoop();
	}
}
