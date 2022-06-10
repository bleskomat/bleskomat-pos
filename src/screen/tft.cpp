#include "screen/tft.h"

namespace {

	TFT_eSPI tft = TFT_eSPI();
	const uint16_t bgColor = TFT_BLACK;
	const int minContrastPercent = 10;
	int currentContrastPercent = 100;
	uint16_t textColor = TFT_WHITE;
	int16_t center_x;
	int16_t center_y;
	std::string currentPaymentQRCodeData;
	bool backlightOff = false;

	typedef std::vector<GFXfont> FontList;

	FontList monospaceFonts = {
		// Ordered from largest (top) to smallest (bottom).
		Courier_Prime_Code28pt7b,
		Courier_Prime_Code24pt7b,
		Courier_Prime_Code22pt7b,
		Courier_Prime_Code20pt7b,
		Courier_Prime_Code18pt7b,
		Courier_Prime_Code16pt7b,
		Courier_Prime_Code14pt7b,
		Courier_Prime_Code12pt7b,
		Courier_Prime_Code10pt7b
	};

	FontList brandFonts = {
		// Ordered from largest (top) to smallest (bottom).
		CheckbookLightning22pt7b,
		CheckbookLightning20pt7b,
		CheckbookLightning18pt7b,
		CheckbookLightning16pt7b,
		CheckbookLightning14pt7b,
		CheckbookLightning12pt7b
	};

	struct BoundingBox {
		int16_t x = 0;
		int16_t y = 0;
		uint16_t w = 0;
		uint16_t h = 0;
	};

	BoundingBox amountTextBBox;

	std::string getAmountFiatCurrencyString(const double &amount) {
		return util::doubleToStringWithPrecision(amount, config::getUnsignedShort("fiatPrecision")) + " " + config::getString("fiatCurrency");
	}

	BoundingBox calculateTextDimensions(const std::string &t_text, const GFXfont font) {
		BoundingBox bbox;
		const char* text = t_text.c_str();
		tft.setTextSize(1);
		tft.setFreeFont(&font);
		const uint16_t textWidth = tft.textWidth(text);
		const uint16_t textHeight = tft.fontHeight();
		bbox.w = textWidth;
		bbox.h = textHeight;
		return bbox;
	}

	GFXfont getBestFitFont(const std::string &text, const FontList fonts, uint16_t max_w = 0, uint16_t max_h = 0) {
		if (max_w == 0) {
			max_w = tft.width();
		}
		if (max_h == 0) {
			max_h = tft.height();
		}
		for (uint8_t index = 0; index < fonts.size(); index++) {
			const GFXfont font = fonts.at(index);
			const BoundingBox bbox = calculateTextDimensions(text, font);
			if (bbox.w <= max_w && bbox.h <= max_h) {
				// Best fit font found.
				return font;
			}
		}
		// Default to last font in list - should be smallest.
		return fonts.back();
	}

	BoundingBox renderText(
		const std::string &t_text,
		const GFXfont font,
		const int16_t x,
		const int16_t y,
		const uint8_t &alignment = MC_DATUM
	) {
		const char* text = t_text.c_str();
		tft.setTextColor(textColor);
		tft.setTextSize(1);
		tft.setFreeFont(&font);
		BoundingBox bbox = calculateTextDimensions(text, font);
		int16_t cursor_x = x;
		int16_t cursor_y = y;
		tft.setTextDatum(alignment);
		tft.drawString(text, cursor_x, cursor_y);
		bbox.x = cursor_x;
		bbox.y = cursor_y;
		return bbox;
	}

	BoundingBox renderQRCode(
		const std::string &t_data,
		const int16_t x,
		const int16_t y,
		const uint16_t &max_w,
		const uint16_t &max_h,
		const bool &center = true
	) {
		BoundingBox bbox;
		try {
			const char* data = t_data.c_str();
			uint8_t version = 1;
			while (version <= 40) {
				const uint16_t bufferSize = qrcode_getBufferSize(version);
				QRCode qrcode;
				uint8_t qrcodeData[bufferSize];
				const int8_t result = qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, data);
				if (result == 0) {
					// QR encoding successful.
					const uint8_t scale = std::min(std::floor(max_w / qrcode.size), std::floor(max_h / qrcode.size));
					const uint16_t w = qrcode.size * scale;
					const uint16_t h = w;
					int16_t box_x = x;
					int16_t box_y = y;
					if (center) {
						box_x -= (w / 2);
						box_y -= (h / 2);
					}
					tft.fillRect(box_x, box_y, w, h, textColor);
					for (uint8_t y = 0; y < qrcode.size; y++) {
						for (uint8_t x = 0; x < qrcode.size; x++) {
							const auto color = qrcode_getModule(&qrcode, x, y) ? bgColor : textColor;
							tft.fillRect(box_x + scale*x, box_y + scale*y, scale, scale, color);
						}
					}
					bbox.x = box_x;
					bbox.y = box_y;
					bbox.w = w;
					bbox.h = h;
					break;
				} else if (result == -2) {
					// Data was too long for the QR code version.
					version++;
				} else if (result == -1) {
					throw std::runtime_error("Unable to detect mode");
				} else {
					throw std::runtime_error("Unknown failure case");
				}
			}
			// Draw a margin around the QR code - to improve readability.
			const uint32_t margin = (tft.height() - bbox.h) / 2;
			tft.fillRect(bbox.x - margin, 0, margin, tft.height(), textColor);
			tft.fillRect(bbox.x, 0, bbox.w, margin, textColor);
			tft.fillRect(bbox.x + bbox.w, 0, margin, tft.height(), textColor);
			tft.fillRect(bbox.x, tft.height() - margin, bbox.w, margin, textColor);
		} catch (const std::exception &e) {
			std::cerr << e.what() << std::endl;
			logger::write("Error while rendering QR code: " + std::string(e.what()), "error");
		}
		return bbox;
	}

	void clearScreen(const bool &reset = true) {
		tft.fillScreen(bgColor);
		if (reset && currentPaymentQRCodeData != "") {
			currentPaymentQRCodeData = "";
		}
	}

	void setContrastLevel(const int &percent) {
		currentContrastPercent = percent;
		const int value = std::ceil((percent * 255) / 100);
		textColor = tft.color565(value, value, value);
		logger::write("Set contrast level to " + std::to_string(percent) + " %");
		if (currentPaymentQRCodeData != "") {
			screen_tft::showPaymentQRCodeScreen(currentPaymentQRCodeData);
		}
	}
}

namespace screen_tft {

	void init() {
		logger::write("Initializing TFT tft...");
		tft.begin();
		tft.setRotation(config::getUnsignedShort("tftRotation"));
		logger::write("TFT display width = " + std::to_string(tft.width()));
		logger::write("TFT display height = " + std::to_string(tft.height()));
		center_x = tft.width() / 2;
		center_y = tft.height() / 2;
		clearScreen();
		setContrastLevel(60);
	}

	void showHomeScreen() {
		clearScreen();
		const std::string logoText = "BLESKOMAT";
		const GFXfont logoFont = getBestFitFont(logoText, brandFonts);
		renderText(logoText, logoFont, center_x, center_y - 6);
		const std::string instructionText = i18n::t("home_instruction");
		renderText(instructionText, Courier_Prime_Code10pt7b, center_x, tft.height() - 16);
	}

	void showEnterAmountScreen(const double &amount) {
		clearScreen();
		const std::string instructionText2 = i18n::t("enter_amount_instruction2");
		const BoundingBox instructionText2_bbox = renderText(instructionText2, Courier_Prime_Code10pt7b, tft.width(), tft.height(), BR_DATUM);
		const std::string instructionText1 = i18n::t("enter_amount_instruction1");
		int16_t instructionText1_y = tft.height();
		int16_t amount_y = center_y - 12;
		if (instructionText1.size() + instructionText2.size() > 16) {
			instructionText1_y -= (instructionText2_bbox.h - 2);
			amount_y -= (instructionText2_bbox.h - 2) / 2;
		}
		renderText(instructionText1, Courier_Prime_Code10pt7b, 0, instructionText1_y, BL_DATUM);
		const std::string amountText = getAmountFiatCurrencyString(amount);
		const GFXfont amountFont = getBestFitFont(amountText, monospaceFonts);
		renderText(amountText, amountFont, center_x, amount_y);
	}

	void showPaymentQRCodeScreen(const std::string &qrcodeData) {
		clearScreen(false);
		const int16_t qr_max_w = tft.width();
		const int16_t qr_max_h = tft.height();
		renderQRCode(qrcodeData, center_x, center_y, qr_max_w, qr_max_h);
		currentPaymentQRCodeData = qrcodeData;
	}

	void showPaymentPinScreen(const std::string &pin) {
		clearScreen();
		const GFXfont pinFont = getBestFitFont(pin, monospaceFonts);
		renderText(pin, pinFont, center_x, center_y);
		const std::string instructionText1 = i18n::t("payment_pin_instruction1");
		renderText(instructionText1, Courier_Prime_Code12pt7b, center_x, 18);
		const std::string instructionText2 = i18n::t("payment_pin_instruction2");
		renderText(instructionText2, Courier_Prime_Code10pt7b, 0, tft.height(), BL_DATUM);
	}

	void adjustContrast(const int &percentChange) {
		const int newContrastPercent = std::max(minContrastPercent, std::min(100, currentContrastPercent + percentChange));
		if (newContrastPercent != currentContrastPercent) {
			setContrastLevel(newContrastPercent);
		}
	}

	void sleep() {
		if (TFT_BL && !backlightOff) {
			logger::write("Turning off TFT backlight");
			digitalWrite(TFT_BL, LOW);
			backlightOff = true;
		}
	}

	void wakeup() {
		if (TFT_BL && backlightOff) {
			logger::write("Turning on TFT backlight");
			digitalWrite(TFT_BL, HIGH);
			backlightOff = false;
		}
	}
}
