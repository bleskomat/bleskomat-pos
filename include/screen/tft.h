#ifndef BLESKOMAT_SCREEN_TFT_H
#define BLESKOMAT_SCREEN_TFT_H

#include "i18n.h"
#include "logger.h"
#include "util.h"

#include <Arduino.h>
#include <qrcode.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "fonts/checkbooklightning_12pt.h"
#include "fonts/checkbooklightning_14pt.h"
#include "fonts/checkbooklightning_16pt.h"
#include "fonts/checkbooklightning_18pt.h"
#include "fonts/checkbooklightning_20pt.h"
#include "fonts/checkbooklightning_22pt.h"
#include "fonts/courier_prime_code_10pt.h"
#include "fonts/courier_prime_code_12pt.h"
#include "fonts/courier_prime_code_14pt.h"
#include "fonts/courier_prime_code_16pt.h"
#include "fonts/courier_prime_code_18pt.h"
#include "fonts/courier_prime_code_20pt.h"
#include "fonts/courier_prime_code_22pt.h"
#include "fonts/courier_prime_code_24pt.h"
#include "fonts/courier_prime_code_28pt.h"

namespace screen_tft {
	void init();
	void showHomeScreen();
	void showEnterAmountScreen(const double &amount);
	void showPaymentQRCodeScreen(const std::string &qrcodeData);
	void showPaymentPinScreen(const std::string &pin);
	void adjustContrast(const int &percentChange);
	void showBatteryPercent(const int &percent);
	void hideBatteryPercent();
	void sleep();
	void wakeup();
}

#endif
