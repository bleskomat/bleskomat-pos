## Usage
#
#   $ make install     # run install process(es)
#   $ make compile     # compile the device firmware
#   $ make upload      # compile then upload the device firmware
#   $ make monitor     # start the serial monitor
#

## Variables
BAUDRATE ?= 115200
DEVICE ?= /dev/ttyACM1
ENV ?= lilygo_ttgo_tdisplay
PLATFORM=espressif32

## Phony targets - these are for when the target-side of a definition
# (such as "install" below) isn't a file but instead just a label. Declaring
# it as phony ensures that it always run, even if a file by the same name
# exists.
.PHONY: install\
compile\
upload\
monitor

install:
	platformio lib install
	platformio platform install ${PLATFORM}

compile:
	platformio run --environment ${ENV}

upload:
	sudo chown ${USER}:${USER} ${DEVICE}
	platformio run --environment ${ENV} --upload-port ${DEVICE} --target upload

monitor:
	sudo chown ${USER}:${USER} ${DEVICE}
	platformio device monitor --baud ${BAUDRATE} --port ${DEVICE}
