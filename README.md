config arduino:
- paste into preferences to install esp32 board
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
- install esp32 board
- set board config to AI thinker esp 32 cam , cpu freq 240 mhz, flash freq 80m, flashmode QIO, partition scheme HUGE APP (3mb NO ota / 1mbps SPIFFS
- flash with right pins 3. and 4. connected

capture images on 1M baud 

install python stuff:
- python -m pip install opencv-python
- python -m pip install pyserial
                                                                                                                          
run grayscaleAck.py
