# co2
Arduino ESP8266 NodeMCU V2 with MH-Z19B &amp; WS2812B Digitale 5050 RGB LED Strip - 60 LEDs 1m

Code based on https://github.com/AntwerpDesignFactory/tjilp

But instead of a piezo buzzer i use a LED strip to indicate the level of Co2 in the room.

Also the measured CO2 level is tracked in a InfluxDB database every 15 seconds.

## Shopping list:
Part|Price|Qtd.|Url
---|---|---|---
Xiao (main controller)|€ 6.5|1|https://www.tinytronics.nl/shop/nl/platforms/seeed-studio/seeed-studio-seeeduino-xiao-cortex-m0-samd21
MH-Z19B (Co2 sensor)|€ 20.0|1|https://www.tinytronics.nl/shop/nl/sensoren/temperatuur-lucht-vochtigheid/winsen-mh-z19b-co2-sensor-met-kabel
Passive Buzzer|€ 0.3|1|https://www.tinytronics.nl/shop/nl/audio/speakers/passieve-buzzer-3-12v-ac-2khz
LED strip*|€ 12.0 of €0.2/st|1|https://www.tinytronics.nl/shop/nl/verlichting/led-strips/led-strips/ws2812b-digitale-5050-rgb-led-strip-60-leds-1m
Usb port|€ 0.5|1|https://www.tinytronics.nl/shop/nl/connectoren/usb/usb-a-connector-diy-male
Diode|€ 0.1|1|https://www.tinytronics.nl/shop/nl/componenten/diode/diode-1n4007
Resistor|€ 0.05|1|https://www.tinytronics.nl/shop/nl/componenten/weerstanden/220%CF%89-weerstand-(led-voorschakelweerstand)
3D prints|€ 0.75|1|See print files in folders.
Screws 3.0 x 10 mm**|€ 0.01|3|https://www.tinytronics.nl/shop/nl/prototyping/montagemateriaal/bout-m3-10mm-draad
Total:| ±30 euro||
