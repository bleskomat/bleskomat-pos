#include "power.h"

namespace {

	const uint8_t readPin = 34;
	float lastMeasuredInputVoltage;
	unsigned long lastMeasuredVoltageTime;
	const unsigned int measureVoltageDebounce = 2500;
	uint64_t wakeupGPIOMask;
	unsigned long lastVoltageCheckTime;
	const unsigned int voltageCheckDelay = 10000;
	float batteryMaxVolts;
	float batteryMinVolts;
	const unsigned int getBatteryPercentDebounce = 10000;
	unsigned long lastGetBatteryPercentTime;
	int lastBatteryPercent = 0;

	float getInputVoltage(const bool &force = false) {
		if (!force && lastMeasuredInputVoltage > 0 && millis() - lastMeasuredVoltageTime < measureVoltageDebounce) {
			return lastMeasuredInputVoltage;
		}
		float readsTotal = 0.00;
		int numReads = 0;
		do {
			readsTotal += (analogRead(readPin) * 1.0);
			delayMicroseconds(10);
		} while (++numReads < 10);// Reduce noise by doing multiple analog reads.
		const float voltage = ((readsTotal / numReads) / 4095.0f) * 2.0f * 3.3f * (1100.0f / 1000.0f);
		lastMeasuredInputVoltage = voltage;
		lastMeasuredVoltageTime = millis();
		return voltage;
	}

	uint64_t stringListToGPIOMask(const std::string &stringList, const char &delimiter = ',') {
		uint64_t mask = 0;
		if (stringList != "") {
			std::istringstream ss(stringList);
			std::string value;
			while (std::getline(ss, value, delimiter)) {
				if (value != "") {
					const uint32_t pinNum = std::stoi(value.c_str());
					if (rtc_gpio_is_valid_gpio((gpio_num_t)pinNum)) {
						const uint64_t pinMask = std::pow(2, pinNum);
						mask += pinMask;
					} else {
						logger::write("Cannot use pin number " + std::to_string(pinNum) + " for wake-up because it is not a valid RTC GPIO", "warn");
					}
				}
			}
		}
		return mask;
	}
}

namespace power {

	void init() {
		wakeupGPIOMask = stringListToGPIOMask(config::getString("keypadColPins"));
		batteryMaxVolts = config::getFloat("batteryMaxVolts");
		batteryMinVolts = config::getFloat("batteryMinVolts");
	}

	void loop() {
		if (millis() - lastVoltageCheckTime > voltageCheckDelay) {
			logger::write("Input Voltage = " + std::to_string(getInputVoltage()), "debug");
			lastVoltageCheckTime = millis();
		}
	}

	bool isUSBPowered() {
		const float voltage = getInputVoltage();
		return voltage > 4.5 || voltage < 1;
	}

	void sleep() {
		logger::write("Going to sleep...");
		delay(1000);
		esp_sleep_enable_ext1_wakeup(wakeupGPIOMask, ESP_EXT1_WAKEUP_ANY_HIGH);
		esp_deep_sleep_start();
	}

	int getBatteryPercent(const bool &force) {
		if (!(batteryMaxVolts > 0) || !(batteryMinVolts > 0)) {
			return 0;
		}
		if (!force && lastGetBatteryPercentTime > 0 && millis() - lastGetBatteryPercentTime < getBatteryPercentDebounce) {
			return lastBatteryPercent;
		}
		lastBatteryPercent = std::min(100,
			std::max(0,
				100 - (int)std::ceil(100 * ((batteryMaxVolts - getInputVoltage()) / (batteryMaxVolts - batteryMinVolts)))
			)
		);
		lastGetBatteryPercentTime = millis();
		return lastBatteryPercent;
	}
}
