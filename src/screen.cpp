#include "screen.h"

namespace {
	std::string currentScreen = "";
}

namespace screen {

	void init() {
		screen_tft::init();
	}

	std::string getCurrentScreen() {
		return currentScreen;
	}

	void showHomeScreen() {
		logger::write("Show screen: Home");
		screen_tft::showHomeScreen();
		currentScreen = "home";
	}

	void showEnterAmountScreen(const double &amount) {
		logger::write("Show screen: Enter Amount");
		screen_tft::showEnterAmountScreen(amount);
		currentScreen = "enterAmount";
	}

	void showPaymentQRCodeScreen(const std::string &qrcodeData) {
		logger::write("Show screen: Payment QR Code");
		screen_tft::showPaymentQRCodeScreen(qrcodeData);
		currentScreen = "paymentQRCode";
	}

	void showPaymentPinScreen(const std::string &pin) {
		logger::write("Show screen: Payment PIN");
		screen_tft::showPaymentPinScreen(pin);
		currentScreen = "paymentPin";
	}

	void adjustContrast(const int &percentChange) {
		screen_tft::adjustContrast(percentChange);
	}

	void showBatteryPercent(const int &percent) {
		screen_tft::showBatteryPercent(percent);
	}

	void hideBatteryPercent() {
		screen_tft::hideBatteryPercent();
	}

	void sleep() {
		screen_tft::sleep();
	}

	void wakeup() {
		screen_tft::wakeup();
	}
}
