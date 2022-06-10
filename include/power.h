#ifndef BLESKOMAT_POWER_H
#define BLESKOMAT_POWER_H

#include "logger.h"
#include "driver/rtc_io.h"
#include <cmath>
#include <string>

namespace power {
	void init();
	void loop();
	bool isUSBPowered();
	void sleep();
}

#endif
