#include "util.h"

namespace {

	int xorEncrypt(uint8_t *output, size_t outlen, uint8_t *key, size_t keylen, uint8_t *nonce, size_t nonce_len, uint32_t pin, uint32_t amount_in_cents) {
		// check we have space for all the data:
		// <variant_byte><len|nonce><len|payload:{pin}{amount}><hmac>
		if (outlen < 2 + nonce_len + 1 + lenVarInt(pin) + 1 + lenVarInt(amount_in_cents) + 8) {
			return 0;
		}
		int cur = 0;
		output[cur] = 1; // variant: XOR encryption
		cur++;
		// nonce_len | nonce
		output[cur] = nonce_len;
		cur++;
		memcpy(output + cur, nonce, nonce_len);
		cur += nonce_len;
		// payload, unxored first - <pin><currency byte><amount>
		int payload_len = lenVarInt(pin) + 1 + lenVarInt(amount_in_cents);
		output[cur] = (uint8_t)payload_len;
		cur++;
		uint8_t *payload = output + cur;                                 // pointer to the start of the payload
		cur += writeVarInt(pin, output + cur, outlen - cur);             // pin code
		cur += writeVarInt(amount_in_cents, output + cur, outlen - cur); // amount
		cur++;
		// xor it with round key
		uint8_t hmacresult[32];
		SHA256 h;
		h.beginHMAC(key, keylen);
		h.write((uint8_t *)"Round secret:", 13);
		h.write(nonce, nonce_len);
		h.endHMAC(hmacresult);
		for (int i = 0; i < payload_len; i++) {
			payload[i] = payload[i] ^ hmacresult[i];
		}
		// add hmac to authenticate
		h.beginHMAC(key, keylen);
		h.write((uint8_t *)"Data:", 5);
		h.write(output, cur);
		h.endHMAC(hmacresult);
		memcpy(output + cur, hmacresult, 8);
		cur += 8;
		// return number of bytes written to the output
		return cur;
	}

	std::string urlEncode(const std::string &value) {
		std::ostringstream escaped;
		escaped.fill('0');
		escaped << std::hex;
		for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
			std::string::value_type c = (*i);
			// Keep alphanumeric and other accepted characters intact
			if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
				escaped << c;
				continue;
			}
			// Any other characters are percent-encoded
			escaped << std::uppercase;
			escaped << '%' << std::setw(2) << int((unsigned char) c);
			escaped << std::nouppercase;
		}
		return escaped.str();
	}
}

namespace util {

	std::string generateRandomPin() {
		// To maintain compatibility with LNPoS/LNURLPoS, use a range of 1000-9999:
		std::string pin = std::to_string(random(1000, 9999));
		return pin;
	}

	std::string createLnurlPay(const double &t_amount, const std::string &t_pin) {
		try {
			// Convert amount to cents:
			const uint32_t amount = std::floor(t_amount * 100);
			const uint32_t pin = std::stoi(t_pin);
			uint8_t nonce[8];
			for (int index = 0; index < 8; index++) {
				nonce[index] = rand() % 256;
			}
			const std::string key = config::getString("apiKey.key");
			const std::string encoding = config::getString("apiKey.encoding");
			uint8_t keyBytes[64];
			size_t keyBytes_len;
			if (encoding == "hex") {
				keyBytes_len = fromHex(key, keyBytes, sizeof(keyBytes));
			} else if (encoding == "base64") {
				keyBytes_len = fromBase64(key, keyBytes, sizeof(keyBytes), BASE64_STANDARD);
			} else if (encoding == "") {
				const char* keyChar = key.c_str();
				const size_t length = strlen(keyChar) + 1;
				const char* start = keyChar;
				const char* end = keyChar + length;
				size_t index = 0;
				for (; start != end; ++start, ++index) {
					keyBytes[index] = (uint8_t)(*start);
				}
				keyBytes_len = key.length();
			}
			uint8_t payload[51];// 51 bytes is max one can get with xor-encryption
			size_t payload_len;
			payload_len = xorEncrypt(payload, sizeof(payload), (uint8_t *)keyBytes, keyBytes_len, nonce, sizeof(nonce), pin, amount);
			return config::getString("callbackUrl") + "?p=" + urlEncode(toBase64(payload, payload_len, BASE64_STANDARD).c_str());
		} catch (const std::exception &e) {
			logger::write("Error while creating lnurl-pay: " + std::string(e.what()), "error");
		}
		return "";
	}

	std::string lnurlEncode(const std::string &text) {
		return Lnurl::encode(text);
	}

	std::string toUpperCase(std::string s) {
		std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
		return s;
	}

	std::vector<float> stringListToFloatVector(const std::string &stringList, const char &delimiter) {
		std::vector<float> items;
		if (stringList != "") {
			std::istringstream ss(stringList);
			std::string value;
			while (std::getline(ss, value, delimiter)) {
				float item = 0;
				if (value != "") {
					item = std::atof(value.c_str());
				}
				items.push_back(item);
			}
		}
		return items;
	}

	std::string doubleToStringWithPrecision(const double &value, const unsigned short &precision) {
		std::ostringstream out;
		out.precision(precision);
		out << std::fixed << value;
		return out.str();
	}
}

