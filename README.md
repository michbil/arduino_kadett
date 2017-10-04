# Opel kadett odometer repair using arduino

This file contains arduino software, that was used to repair Opel Kadett GSI LCD odometer. Original odometer was mechanical,
and it's platic gear was destroyed because of aging. So I removed original mechanical odometer unit and replaced it with 1602 LCD screen.

(see photos folder)

Connection diagram:

SPEED PULSE: PIN 2
 RESET BUTTON: PIN 8
 The circuit (make sure to power the LCD):
 * LCD RS pin to digital pin 12
 * LCD E pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 9
 * LCD Ve pin (contrast) to digital pin 6
 
 
 ![Connection diagram](https://raw.githubusercontent.com/michbil/arduino_kadett/master/photos/thumb.jpg)
