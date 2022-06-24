#include "main.h"

uint16_t amountCentsDivisor = 1;
unsigned short maxNumKeysPressed = 12;
unsigned int sleepModeDelay;

std::string pin = "";
std::string qrcodeData = "";
std::string keysBuffer = "";
const std::string keyBufferCharList = "0123456789";

void appendToKeyBuffer(const std::string &key) {
	if (keyBufferCharList.find(key) != std::string::npos) {
		keysBuffer += key;
	}
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
	if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) {
		// Waking up from deep sleep...
		cache::init();
		const std::string lastScreen = cache::getString("lastScreen");
		pin = cache::getString("pin");
		qrcodeData = cache::getString("qrcodeData");
		keysBuffer = cache::getString("keysBuffer");
		cache::end();
		logger::write("Waking up... ");
		if (lastScreen == "home") {
			screen::showHomeScreen();
		} else if (lastScreen == "enterAmount") {
			screen::showEnterAmountScreen(keysToAmount(keysBuffer));
		} else if (lastScreen == "paymentQRCode") {
			screen::showPaymentQRCodeScreen(qrcodeData);
		} else if (lastScreen == "paymentPin") {
			screen::showPaymentPinScreen(pin);
		}
	}
}

unsigned long lastActivityTime = millis();
bool isFakeSleeping = false;

void handleSleepMode() {
	if (sleepModeDelay > 0) {
		if (millis() - lastActivityTime > sleepModeDelay) {
			if (power::isUSBPowered()) {
				if (!isFakeSleeping) {
					// The battery does not charge while in deep sleep mode.
					// So let's just turn off the screen instead.
					screen::sleep();
					isFakeSleeping = true;
				}
			} else {
				cache::init();
				cache::save("pin", pin);
				cache::save("keysBuffer", keysBuffer);
				cache::save("qrcodeData", qrcodeData);
				cache::save("lastScreen", screen::getCurrentScreen());
				cache::end();
				power::sleep();
			}
		} else if (isFakeSleeping) {
			screen::wakeup();
			const std::string lastScreen = screen::getCurrentScreen();
			if (lastScreen == "home") {
				screen::showHomeScreen();
			} else if (lastScreen == "enterAmount") {
				screen::showEnterAmountScreen(keysToAmount(keysBuffer));
			} else if (lastScreen == "paymentQRCode") {
				screen::showPaymentQRCodeScreen(qrcodeData);
			} else if (lastScreen == "paymentPin") {
				screen::showPaymentPinScreen(pin);
			}
			isFakeSleeping = false;
		}
	}
}

void runAppLoop() {
	power::loop();
	handleSleepMode();
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
				appendToKeyBuffer(keyPressed);
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
				qrcodeData = "";
				pin = util::generateRandomPin();
				const std::string signedUrl = util::createLnurlPay(amount, pin);
				const std::string encoded = util::lnurlEncode(signedUrl);
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
				appendToKeyBuffer(keyPressed);
				logger::write("keysBuffer = " + keysBuffer);
				screen::showEnterAmountScreen(keysToAmount(keysBuffer));
			}
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
	if (power::isUSBPowered()) {
		screen::hideBatteryPercent();
	} else {
		screen::showBatteryPercent(power::getBatteryPercent());
	}
}

void loop() {
	logger::loop();
	jsonRpc::loop();
	if (!jsonRpc::hasPinConflict() || !jsonRpc::inUse()) {
		runAppLoop();
	}
}
