#!/bin/sh

# Get the specified file from cargs
file=$1
path="$file/$file.ino"

# Check if the file exists
if [ ! -f $path ]; then
    echo "File not found!"
    exit 1
fi

# Compile and upload the file
echo "Compiling and uploading $file"
arduino-cli compile --fqbn esp8266:esp8266:d1_wroom_02 $path

# Check if the compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful!"
else
    echo "Compilation failed!"
    exit 1
fi

arduino-cli upload -v -p /dev/cu.usbserial-FTB6SPL3 --fqbn esp8266:esp8266:d1_wroom_02 $path

# Check if the upload was successful
if [ $? -eq 0 ]; then
    echo "Upload successful!"
else
    echo "Upload failed"
fi

# Exit
echo "Exiting..."
exit 0