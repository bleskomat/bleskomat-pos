#ifndef BLESKOMAT_UTIL_H
#define BLESKOMAT_UTIL_H

#include "config.h"

#include <Arduino.h>
#include <lnurl.h>
#include <Hash.h>
#include <Conversion.h>

#include <chrono>
#include <cmath>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace util {

	// This is needed to use char* as the key in std::map.
	struct MapCharPointerComparator {
		bool operator()(char const *a, char const *b) const {
			return std::strcmp(a, b) < 0;
		}
	};

	std::string generateRandomPin();
	std::string createLnurlPay(const double &t_amount, const std::string &t_pin);
	std::string lnurlEncode(const std::string &text);
	std::string toUpperCase(std::string s);
	std::vector<float> stringListToFloatVector(const std::string &stringList, const char &delimiter = ',');
	std::string doubleToStringWithPrecision(const double &value, const unsigned short &precision = 6);
}

#endif
