# bleskomat-pos

The Bleskomat POS is an offline point-of-sale terminal device for the Bitcoin Lightning Network. This repository contains the firmware source, scripts to compile and upload the firmware to a hardware device, and basic instructions.

* [Software Requirements](#software-requirements)
* [Installing Libraries and Dependencies](#installing-libraries-and-dependencies)
* [Compiling and Uploading to Device](#compiling-and-uploading-to-device)
* [Generate Font Header Files](#generate-font-header-files)
* [Support](#support)
* [Changelog](#changelog)
* [License](#license)
* [Trademark](#trademark)


## Software Requirements

* [make](https://www.gnu.org/software/make/)
* [PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/)
	* Version 5 or newer
	* Only the CLI ("Core") is required


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
./tools/Adafruit-GFX-Library/fontconvert ./assets/fonts/Checkbook/Checkbook.ttf 30 > ./include/fonts/checkbook_30pt.h
```
Don't forget to add header guards to the new file - e.g.:
```c
#ifndef BLESKOMAT_FONTS_CHECKBOOK_30PT_H
#define BLESKOMAT_FONTS_CHECKBOOK_30PT_H

// ...

#endif
```
Then include the new font header file near the others in the `./include/screen/tft.h` file:
```c
// ...
#include "fonts/checkbook_30pt.h"
// ...
```


## Support

Need some help? Join us in the official [Telegram group](https://t.me/bleskomat) or send us an email at [support@bleskomat.com](mailto:support@bleskomat.com) and we will try our best to respond in a reasonable time. If you have a feature request or bug to report, please [open an issue](https://github.com/bleskomat/bleskomat-pos/issues) in this project repository.


## Changelog

See [CHANGELOG.md](https://github.com/bleskomat/bleskomat-pos/blob/master/CHANGELOG.md)


## License

The project is licensed under the [GNU General Public License v3 (GPL-3)](https://tldrlegal.com/license/gnu-general-public-license-v3-(gpl-3)):
> You may copy, distribute and modify the software as long as you track changes/dates in source files. Any modifications to or software including (via compiler) GPL-licensed code must also be made available under the GPL along with build & install instructions.


## Trademark

"Bleskomat" is a registered trademark. You are welcome to hack, fork, build, and use the source code and instructions found in this repository. However, the right to use the name "Bleskomat" with any commercial products or services is withheld and reserved for the trademark owner.
