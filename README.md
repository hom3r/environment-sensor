# Environment sensor

Environment sensor project measuring air temperature and humidity.

It is made for on TTGO-Display-T (with ESP32 chip and TFT display), but it can be modified to run on Arduino,
NodeMCU or any other similar device.

## Setup
First rename (or copy) `config.h.example` to `config.h` and set your WiFi credentials along with the ThingSpeak API key (lines 2, 3 and 6). Then set up your Arduino IDE for TTGO module, install required libraries and upload the code.

Warning: Be sure to keep the `config.h` in the `.gitignore` file and not to commit it anywhere to prevent your credentials leakage!

You can change the other constants in your `config.h` to change the measurement frequency, how often the data should be sent to the server along with settings to display the data on TFT. Be aware that only font supporting the degree (°) I found is the font number 1.

TODO: add TTGO setup
