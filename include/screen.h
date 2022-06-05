#ifndef BLESKOMAT_SCREEN_H
#define BLESKOMAT_SCREEN_H

#include "config.h"
#include "logger.h"
#include "screen/tft.h"

namespace screen {
	void init();
	std::string getCurrentScreen();
	void showHomeScreen();
	void showEnterAmountScreen(const double &amount);
	void showPaymentQRCodeScreen(const std::string &qrcodeData);
	void showPaymentPinScreen(const std::string &pin);
}

#endif
