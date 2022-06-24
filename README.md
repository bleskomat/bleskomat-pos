# Bleskomat POS

The Bleskomat POS is an offline point-of-sale terminal device for the Bitcoin Lightning Network. This repository includes the open-source firmware, how-to instructions to build your own hardware device, and scripts to compile the firmware from source.

You can buy the [pre-assembled Bleskomat POS](https://shop.bleskomat.com/product/bleskomat-pos) from the official Bleskomat shop - includes battery, 3D-printed case, and subscription to the Bleskomat Platform. Alternatively, you can buy the components to build your own from the official [Lilygo shop](https://aliexpress.com/item/1005003589706292.html).

The Bleskomat POS must be paired with a server to facilitate Lightning Network payments on its behalf; see the options below:
* [Bleskomat Platform](https://platform.bleskomat.com) - non-custodial, requires a subscription
* [bleskomat-server](https://github.com/bleskomat/bleskomat-server) - non-custodial, open-source, self-hosted solution
* [lnbits](https://github.com/lnbits/lnbits-legend) via the LNURLDevice extension - open-source, self-hosted and possible to use custodial instances hosted by others; public instances of lnbits:
	* [legend.lnbits.com](https://legend.lnbits.com) - unstable, don't leave funds on this instance for very long

The rest of this document details the hardware and software requirements, how to build the hardware yourself, and instructions for compiling and uploading the firmware from source.

* [Requirements](#requirements)
	* [Hardware Requirements](#hardware-requirements)
	* [Software Requirements](#software-requirements)
* [Building the Hardware Device](#building-the-hardware-device)
	* [Lilygo T-Display Kit](#lilygo-t-display-key)
	* [ESP32 Devkit](#esp32-devkit)
		* [Wiring the TFT Display](#wiring-the-tft-display)
		* [Wiring the Membrane Keypad](#wiring-the-membrane-keypad)
* [Installing Libraries and Dependencies](#installing-libraries-and-dependencies)
* [Compiling and Uploading to Device](#compiling-and-uploading-to-device)
* [Generate Font Header Files](#generate-font-header-files)
* [Configuring the Device](#configuring-the-device)
	* [List of Configuration Options](#list-of-configuration-options)
	* [Browser-Based Configuration Tool](#browser-based-configuration-tool)
	* [Hard-Coded Configuration](#hard-coded-configuration)
* [Changelog](#changelog)
* [Support](#support)
* [License](#license)
* [Trademark](#trademark)


## Requirements

This section includes information about the software and hardware requirements needed to build this project.


### Hardware Requirements

Components needed to build your own Bleskomat POS:
* Lilygo TTGO T-Display kit - this includes the following:
	* Lilygo T-Display Module
	* Keyboard
	* USB-C cable
	* JST connector (for battery)
	* With or without 3D-printed case
* 3.7V Lipo battery with built-in over/under charge protection

Alternative build components using an __ESP32 Devkit__:
* ESP32 Devkit
* 1.8" TFT display
* 2 x 400-pin breadboards
* Membrane keypad (4x4 or 4x3)
* Standard USB to micro USB cable

Equipment/tools needed:
* Soldering iron
* Multimeter


### Software Requirements

* [make](https://www.gnu.org/software/make/)
* [PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/)
	* Version 5 or newer
	* Only the CLI ("Core") is required


## Building the Hardware Device

Before proceeding, be sure that you have all the project's [hardware requirements](#hardware-requirements).

Step-by-step build process for the hardware device.


### Lilygo T-Display Kit

If using a battery, insert the battery's JST connector into the JST socket on the under-side of the T-Display devkit board. Insert the T-Display devkit board's pins into the keyboard module's sockets - they should line-up perfectly. Solder the pins so that the devkit is solidly connected to the keyboard.


### ESP32 Devkit

One breadboard is not large enough to accommodate all the pins of the ESP32 devkit due to the width of the devkit. This is why we recommend to connect two breadboards together.

Remove one of the power rails from one of the breadboards. Use the notches on the sides of the breadboards to connect them together length-wise.

Insert the ESP32 devkit into the pin holes of the new, combined breadboard.

![](docs/esp32devkit-build-breadboard-and-esp32-devkit.png)

Familiarize yourself with the ESP32 devkit's pinout reference below.

#### ESP32 devkit pinout

![](docs/ESP32-devkit-v1-board-pinout-36-gpio-pins.jpg)


### Wiring the TFT Display

Insert the pins of the TFT display module into the breadboard where you have space available.

![](docs/esp32devkit-build-tft-display.png)

Use the table below to connect the ESP32 devkit to the TFT display module.

|  ESP32       | TFT         |
|--------------|-------------|
| VIN (V5)     | VCC         |
| GND          | GND         |
| GPIO22       | CS          |
| GPIO4        | RESET (RST) |
| GPIO2        | AO (RS)     |
| GPIO23       | SDA         |
| GPIO18       | SCK (CLK)   |
| 3V3          | LED (NC)    |

Notes on pin naming:
* There are boards where `GPIXXX` are marked as `GXX` instead of `DXX`.
* The `G23` may be there **twice** - the correct one is next to `GND`.
* Some boards have typos so a bit of guess-and-check is necessary sometimes.

![](docs/esp32devkit-build-tft-display-with-wires.png)

Refer to the [ESP32 devkit pinout](#esp32-devkit-pinout) for help identifying the pins on your ESP32.


### Wiring the Membrane Keypad

Use the table below to connect the ESP32 devkit to the membrane keypad.

| ESP32       | Keypad   |
|-------------|----------|
| GPIO13      | R1       |
| GPIO14      | R2       |
| GPIO26      | R3       |
| GPIO25      | R4       |
| GPIO33      | C1       |
| GPIO32      | C2       |
| GPIO15      | C3       |
| GPIO21      | C4       |

Refer to the [ESP32 devkit pinout](#esp32-devkit-pinout) for help identifying the pins on your ESP32.


## Installing Libraries and Dependencies

Before proceeding, be sure that you have all the project's [software requirements](#software-requirements).

Use make to install libraries and dependencies needed to build the firmware:
```bash
make install
```
* The firmware's dependencies are defined in its platformio.ini file located at `./platformio.ini`

If while developing you need to install a new library, use the following as a guide:
```bash
platformio lib install LIBRARY_NAME[@VERSION]
```
You can find PlatformIO's libraries repository [here](https://platformio.org/lib).


## Compiling and Uploading to Device

To compile the firmware (without uploading to a device):
```bash
make compile
```

To compile and upload to your device:
```bash
make upload DEVICE=/dev/ttyACM1
```
The device path for your operating system might be different. If you receive a "Permission denied" error about `/dev/ttyACM1` then you will need to set permissions for that file on your system:
```bash
sudo chown $USER:$USER /dev/ttyACM1
```

To open the serial monitor:
```bash
make monitor DEVICE=/dev/ttyACM1
```
Again the device path here could be different for your operating system.


## Generate Font Header Files

The project already includes several font header files in various sizes. But in the case that you would like to add more or use a different font entirely, you can use the fontconvert tool from the [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library), included with this project.

Follow the steps below to compile the fontconvert tool:
```
cd ./tools/Adafruit-GFX-Library/fontconvert;
make fontconvert
```
Then to generate a new font header file for your own .ttf font:
```bash
./tools/Adafruit-GFX-Library/fontconvert/fontconvert ./assets/fonts/CheckbookLightning/CheckbookLightning.ttf 30 > ./include/fonts/checkbooklightning_30pt.h
```
Don't forget to add header guards to the new file - e.g.:
```c
#ifndef BLESKOMAT_FONTS_CHECKBOOKLIGHTNING_30PT_H
#define BLESKOMAT_FONTS_CHECKBOOKLIGHTNING_30PT_H

// ...

#endif
```
Then include the new font header file near the others in the `./include/screen/tft.h` file:
```c
// ...
#include "fonts/checkbooklightning_30pt.h"
// ...
```


## Configuring the Device

It is possible to configure the device via the following methods:
* [Browser-Based Configuration Tool](#browser-based-configuration-tool)
* [Hard-Coded Configuration](#hard-coded-configuration)


### List of Configuration Options

The following is a list of possible configuration options for the Bleskomat POS:
* `apiKey.key` - The API key secret that is used to generate cryptographic signatures and encrypted payloads.
* `apiKey.encoding` - The explicit encoding of the API key secret. This can be "hex", "base64", or empty-string (e.g "") to mean no encoding. When generating a new API key on the server, it will store the encoding along with the ID and secret.
* `callbackUrl` - The LNURL server base URL plus endpoint path. Example:
	* `https://p.bleskomat.com/pay/<apiKey.id>`
* `uriSchemaPrefix` - The URI schema prefix for LNURLs generated by the device. It has been discovered that some wallet apps mistakenly only support lowercase URI schema prefixes. Uppercase is better because when encoded as a QR code, the generated image is less complex and so easier to scan. Set this config to empty-string (e.g `uriSchemaPrefix=`) to not prepend any URI schema prefix.
* `fiatCurrency` - The fiat currency symbol for which the device is configured; see [ISO 4217](https://en.wikipedia.org/wiki/ISO_4217).
* `fiatPrecision` - The number of digits to the right of the decimal point when rendering fiat currency amounts.
* `keypadRowPins` - Comma-separated list of GPIO numbers for the keypad's row pins.
* `keypadColPins` - Comma-separated list of GPIO numbers for the keypad's column pins.
* `keypadCharList` - The keypad character list read left-to-right, top-to-bottom. Examples:
	* T-Display Keyboard = `"123456789*0#"`
	* Membrane Keypad (4x3) = `"123456789*0#"`
	* Membrane Keypad (4x4) = `"123A456B789C*0#D"`
* `locale` - The locale used when rendering text to the screen. Current supported languages:
	* "cs" - Czech
	* "de" - German
	* "en" - English (default)
	* "es" - Spanish
* `sleepModeDelay` - Number of milliseconds of inactivity to wait before the device will enter power-saving mode.
* `contrastLevel` - The contrast level between text and background colors. If the ambient light level is low, decreasing the contrast can improve scannability of QR codes.
* `tftRotation` The orientation of the TFT display. This is useful to allow different positions of the display. The possible rotation values are:
	* 0 = 0 degrees
	* 1 = 90 degrees
	* 2 = 180 degrees
	* 3 = 270 degrees
	* 3 = 270 degrees
* `logLevel` - Possible values:
	* `trace` - everything
	* `debug`
	* `info` - default
	* `warn`
	* `error`
	* `none` - nothing


### Browser-Based Configuration Tool

The Bleskomat Platform provides a [browser-based configuration tool](https://platform.bleskomat.com/serial) to upload pre-built device firmware, view real-time log output, update device configurations, run JSON-RPC serial commands, and more.


### Hard-Coded Configuration

Hard-coded configurations can be set by modifying the source file [config.cpp](https://github.com/bleskomat/bleskomat-pos/blob/master/src/config.cpp#L128).

Each time you make changes to the hard-coded configurations, you will need to re-compile and flash the ESP32's firmware.


## Changelog

See [CHANGELOG.md](https://github.com/bleskomat/bleskomat-pos/blob/master/CHANGELOG.md)


## Support

Need some help? Join us in the official [Telegram group](https://t.me/bleskomat) or send us an email at [support@bleskomat.com](mailto:support@bleskomat.com) and we will try our best to respond in a reasonable time. If you have a feature request or bug to report, please [open an issue](https://github.com/bleskomat/bleskomat-pos/issues) in this project repository.


## License

The project is licensed under the [GNU General Public License v3 (GPL-3)](https://tldrlegal.com/license/gnu-general-public-license-v3-(gpl-3)):
> You may copy, distribute and modify the software as long as you track changes/dates in source files. Any modifications to or software including (via compiler) GPL-licensed code must also be made available under the GPL along with build & install instructions.


## Trademark

"Bleskomat" is a registered trademark. You are welcome to hack, fork, build, and use the source code and instructions found in this repository. However, the right to use the name "Bleskomat" with any commercial products or services is withheld and reserved for the trademark owner.
