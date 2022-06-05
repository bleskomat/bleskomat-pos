#include "keypad.h"

// https://github.com/Chris--A/Keypad

namespace {
	enum class State {
		uninitialized,
		initialized,
		failed
	};
	State state = State::uninitialized;
	std::vector<byte> keypadRowPins;
	std::vector<byte> keypadColPins;
	Keypad* keypadInstance;
	byte* rowPins;
	byte* colPins;
	const uint8_t numRows = 4;
	const uint8_t numCols = 3;
	char keys[numRows][numCols] = {
		{'1', '2', '3'},
		{'4', '5', '6'},
		{'7', '8', '9'},
		{'*', '0', '#'}
	};
	char* userKeymap = makeKeymap(keys);

	std::vector<byte> stringListToByteVector(const std::string &stringList, const char &delimiter = ',') {
		std::vector<byte> items;
		if (stringList != "") {
			std::istringstream ss(stringList);
			std::string value;
			while (std::getline(ss, value, delimiter)) {
				byte item = 0;
				if (value != "") {
					item = (byte)std::stoi(value.c_str());
				}
				items.push_back(item);
			}
		}
		return items;
	}
}

namespace keypad {

	void init() {
		keypadRowPins = stringListToByteVector(config::getString("keypadRowPins"));
		keypadColPins = stringListToByteVector(config::getString("keypadColPins"));
	}

	void loop() {
		if (state == State::uninitialized) {
			logger::write("Initializing keypad...");
			if (keypadRowPins.size() != numRows) {
				logger::write("Cannot initialize keypad: \"keypadRowPins\" must define " + std::to_string(numRows) + " pins", "warn");
				state = State::failed;
			} else if (keypadColPins.size() != numCols) {
				logger::write("Cannot initialize keypad: \"keypadColPins\" must define " + std::to_string(numCols) + " pins", "warn");
				state = State::failed;
			} else {
				rowPins = keypadRowPins.data();
				colPins = keypadColPins.data();
				keypadInstance = new Keypad(userKeymap, rowPins, colPins, numRows, numCols);
				state = State::initialized;
			}
		}
	}

	std::string getPressedKey() {
		if (state != State::initialized) {
			return "";
		}
		const char key = keypadInstance->getKey();
		if (key == NO_KEY) {
			return "";
		}
		std::cout << "key = " << key << std::endl;
		return std::string() + key;
	}
}
