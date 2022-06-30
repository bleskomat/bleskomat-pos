#!/bin/bash

# Fail and exit on errors:
set -e

thisDir=$(realpath $(dirname "$0"))
outDir="${thisDir}/../include/fonts"

fontconvert="${thisDir}/../tools/Adafruit-GFX-Library/fontconvert/fontconvert"
if [ ! -f "$fontconvert" ]; then
	echo "fontconvert tool not found"
	exit 1
fi

fontFilePath=$(realpath "$1")
if [ ! -f "$fontFilePath" ]; then
	echo "Font file does not exist"
	exit 1
fi

charRange="$2"
if [[ ! $charRange =~ ^[0-9]+-[0-9]+$ ]]; then
	echo "Character range must contain numbers and hyphen only. Example: ${0} ./path/to/font.ttf 32-127 10,12,14"
	exit 1
fi
# Set the delimiter then split string into an array:
IFS='-'
read -ra charRangeArray <<< "$charRange"
firstchar="${charRangeArray[0]}"
lastchar="${charRangeArray[1]}"

fontSizes="$3"
if [[ $fontSizes =~ [^0-9,] ]]; then
	echo "Font sizes must contain numbers and commas only. Example: ${0} ./path/to/font.ttf 32-127 10,12,14"
	exit 1
fi
# Set the delimiter then split string into an array:
IFS=','
read -ra fontSizesArray <<< "$fontSizes"

for fontSize in "${fontSizesArray[@]}"; do
	fontName="$(basename "${fontFilePath}" | cut -f 1 -d '.' | tr -s ' ' | tr ' ' '_' | tr '-' '_')_${fontSize}pt"
	fontNameLower=$(echo -n "${fontName}" | tr '[:upper:]' '[:lower:]')
	fontNameUpper=$(echo -n "${fontName}" | tr '[:lower:]' '[:upper:]')
	outFilePath=$(realpath "${outDir}/${fontNameLower}.h")
	${fontconvert} "${fontFilePath}" ${fontSize} ${firstchar} ${lastchar} > ${outFilePath}
	headerFileBytes="$(cat ${outFilePath})"
	headerGuard="BLESKOMAT_FONTS_${fontNameUpper}_H"
cat > "$outFilePath" <<EOF
#ifndef ${headerGuard}
#define ${headerGuard}

${headerFileBytes}

#endif
EOF
	echo "Created ${outFilePath}"
done
