#ifndef BLESKOMAT_KEYPAD_H
#define BLESKOMAT_KEYPAD_H

#include "config.h"
#include <Keypad.h>
#include <string>
#include <vector>

namespace keypad {
	void init();
	void loop();
	std::string getPressedKey();
}

#endif
