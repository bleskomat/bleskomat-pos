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
		Courier_Prime_Code28pt8b,
		Courier_Prime_Code24pt8b,
		Courier_Prime_Code22pt8b,
		Courier_Prime_Code20pt8b,
		Courier_Prime_Code18pt8b,
		Courier_Prime_Code16pt8b,
		Courier_Prime_Code14pt8b,
		Courier_Prime_Code12pt8b,
		Courier_Prime_Code10pt8b,
		Courier_Prime_Code9pt8b,
		Courier_Prime_Code8pt8b,
		Courier_Prime_Code7pt8b,
		Courier_Prime_Code6pt8b
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
	BoundingBox batteryPercentBBox;

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
		const uint16_t &color,
		const int16_t x,
		const int16_t y,
		const uint8_t &alignment = MC_DATUM
	) {
		const char* text = t_text.c_str();
		tft.setTextColor(color);
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
			uint8_t scale = 1;
			while (version <= 40) {
				const uint16_t bufferSize = qrcode_getBufferSize(version);
				QRCode qrcode;
				uint8_t qrcodeData[bufferSize];
				const int8_t result = qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, data);
				if (result == 0) {
					// QR encoding successful.
					scale = std::min(std::floor(max_w / qrcode.size), std::floor(max_h / qrcode.size));
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
			// Draw a border around the QR code - to improve readability.
			const uint8_t margin_x = std::min(scale, (uint8_t)std::floor((tft.width() - bbox.w) / 2));
			const uint8_t margin_y = std::min(scale, (uint8_t)std::floor((tft.height() - bbox.h) / 2));
			const uint16_t border_x = bbox.x - margin_x;
			const uint16_t border_y = bbox.y - margin_y;
			tft.fillRect(border_x, border_y, margin_x, bbox.h + (margin_y * 2), textColor);// left
			tft.fillRect(bbox.x + bbox.w, border_y, margin_x, bbox.h + (margin_y * 2), textColor);// right
			tft.fillRect(bbox.x, border_y, bbox.w, margin_y, textColor);// top
			tft.fillRect(bbox.x, bbox.y + bbox.h, bbox.w, margin_y, textColor);// bottom
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
		batteryPercentBBox.x = 0;
		batteryPercentBBox.y = 0;
		batteryPercentBBox.w = 0;
		batteryPercentBBox.h = 0;
	}

	void setContrastLevel(const int &percent) {
		currentContrastPercent = percent;
		const int value = std::ceil((percent * 255) / 100);
		textColor = tft.color565(value, value, value);
		const std::string percentStr = std::to_string(percent);
		config::saveConfiguration("contrastLevel", percentStr);
		logger::write("Set contrast level to " + percentStr + " %");
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
		setContrastLevel(config::getUnsignedInt("contrastLevel"));
	}

	void showHomeScreen() {
		clearScreen();
		const std::string logoText = "BLESKOMAT";
		const GFXfont logoFont = getBestFitFont(logoText, brandFonts);
		const BoundingBox logoBBox = renderText(logoText, logoFont, textColor, center_x, center_y - 6);
		const std::string instructionText = i18n::t("home_instruction");
		const GFXfont instructionFont = getBestFitFont(instructionText, monospaceFonts, (tft.width() * 10) / 7, logoBBox.h / 2);
		renderText(instructionText, instructionFont, textColor, center_x, tft.height(), BC_DATUM);
	}

	void showEnterAmountScreen(const double &amount) {
		clearScreen();
		const std::string instructionText1 = i18n::t("enter_amount_instruction1");
		const std::string instructionText2 = i18n::t("enter_amount_instruction2");
		const GFXfont instructionFont = getBestFitFont(instructionText1 + " " + instructionText2, monospaceFonts, tft.width());
		const BoundingBox instructionText2_bbox = renderText(instructionText2, instructionFont, textColor, tft.width(), tft.height(), BR_DATUM);
		int16_t instructionText1_y = tft.height();
		int16_t amount_y = center_y - 12;
		if (instructionText1.size() + instructionText2.size() > 16) {
			instructionText1_y -= (instructionText2_bbox.h - 2);
			amount_y -= (instructionText2_bbox.h - 2) / 2;
		}
		renderText(instructionText1, instructionFont, textColor, 0, instructionText1_y, BL_DATUM);
		const std::string amountText = getAmountFiatCurrencyString(amount);
		const GFXfont amountFont = getBestFitFont(amountText, monospaceFonts);
		renderText(amountText, amountFont, textColor, center_x, amount_y);
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
		const BoundingBox pinBBox = renderText(pin, pinFont, textColor, center_x, center_y - 2);
		const std::string instructionText1 = i18n::t("payment_pin_instruction1");
		const GFXfont instructionFont1 = getBestFitFont(instructionText1, monospaceFonts, tft.width(), pinBBox.h / 2);
		const std::string instructionText2 = i18n::t("payment_pin_instruction2");
		const GFXfont instructionFont2 = getBestFitFont(instructionText2, monospaceFonts, tft.width() / 2, pinBBox.h / 2);
		renderText(instructionText1, instructionFont1, textColor, center_x, 0, TC_DATUM);
		renderText(instructionText2, instructionFont2, textColor, 0, tft.height(), BL_DATUM);
	}

	void adjustContrast(const int &percentChange) {
		const int newContrastPercent = std::max(minContrastPercent, std::min(100, currentContrastPercent + percentChange));
		if (newContrastPercent != currentContrastPercent) {
			setContrastLevel(newContrastPercent);
		}
	}

	void showBatteryPercent(const int &percent) {
		if (currentPaymentQRCodeData == "" && !(batteryPercentBBox.w > 0)) {
			const std::string percentText = std::to_string(percent) + "%";
			uint16_t color;
			if (percent >= 66) {
				color = 0x1983;// green
			} else if (percent >= 33) {
				color = 0x4143;// orange
			} else {
				color = 0x3041;// red
			}
			batteryPercentBBox = renderText(percentText, Courier_Prime_Code10pt8b, color, tft.width(), 0, TR_DATUM);
		}
	}

	void hideBatteryPercent() {
		if (batteryPercentBBox.w > 0) {
			tft.fillRect(batteryPercentBBox.x, batteryPercentBBox.y, batteryPercentBBox.w, batteryPercentBBox.h, bgColor);
			batteryPercentBBox.x = 0;
			batteryPercentBBox.y = 0;
			batteryPercentBBox.w = 0;
			batteryPercentBBox.h = 0;
		}
	}

	void sleep() {
		if (TFT_BL && !backlightOff) {
			logger::write("Turning off TFT backlight");
			digitalWrite(TFT_BL, LOW);
			backlightOff = true;
		} else {
			clearScreen();
		}
	}

	void wakeup() {
		if (TFT_BL && backlightOff) {
			logger::write("Turning on TFT backlight");
			digitalWrite(TFT_BL, HIGH);
			backlightOff = false;
		} else {
			clearScreen();
		}
	}
}
