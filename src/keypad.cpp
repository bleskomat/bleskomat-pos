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
	std::string keypadCharList;
	Keypad* keypadInstance;
	char* userKeymap;

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
		keypadCharList = config::getString("keypadCharList");
	}

	void loop() {
		if (state == State::uninitialized) {
			logger::write("Initializing keypad...");
			if (!(keypadRowPins.size() > 0)) {
				logger::write("Cannot initialize keypad: \"keypadRowPins\" not set", "warn");
				state = State::failed;
			} else if (!(keypadColPins.size() > 0)) {
				logger::write("Cannot initialize keypad: \"keypadColPins\" not set", "warn");
				state = State::failed;
			} else {
				userKeymap = makeKeymap(keypadCharList.c_str());
				keypadInstance = new Keypad(userKeymap, keypadRowPins.data(), keypadColPins.data(), keypadRowPins.size(), keypadColPins.size());
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
		return std::string() + key;
	}
}
