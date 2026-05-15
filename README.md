# RPI_PicoSigner
# Airgapped Bitcoin transaction signing device
# Using RPI Pico RP2040, OV7670 and ST7789
# ** Not finished **
## by Klaus Fensterseifer - PY2KLA

![PicoSigner](Files/RPI_PicoSigner0.jpg)
<br>
<br>
This project is an attempt to create a Bitcoin transaction signing device similar to SeedSigner (https://github.com/SeedSigner/seedsigner), but using a Raspberry Pi Pico without access to Wi-Fi, Bluetooth, or any other means of communication besides a wired USB connection.<br>

It uses an RPI Pico, an OV7670 camera, and an ST7789 display, all low-cost components.<br>

The idea is to create a project similar to SeedSigner, but simpler.<br>
<br>

The main challenge of this project is using the limited memory available on the RPI Pico to capture image data from the camera for QR code scanning. In addition, implementing the Bitcoin cryptographic functions and the user interface also requires considerable effort.<br>
<br>

## I confess...

I confess that I was more interested in seeing whether I could make the camera work with the RPI Pico using PIO, learn more about RGB color formats and scanning QR codes, than in completing the entire project.<br>

Looking at it this way, I achieved my goal: the hardware interface part is done, and now I just need to finish the application layer using the available data.<br>
<br>

At this point, the project is already capable of reading data from the camera, scanning QR codes, and interacting with the user through switches and the display.<br>
<br>


## PicoSigner schematic
![PicoSigner](Files/Pico_OV7670_ST7789.png)


## PicoSigner top (ugly assembly)
![PicoSigner](Files/RPI_PicoSigner_top.jpg)

## PicoSigner botton (ugly assembly)
![PicoSigner](Files/RPI_PicoSigner_bot.jpg)


## Software
It uses the Arduino IDE to compile.
 - Board: Arduino Raspberry Pi Pico/RP2040/RP2350 by  Earle F. Philhower, III / Raspberry Pi Pico<br>
<br>
Arduino Libraries used: 
 - Crypto by Rhys Weatherley
 - QRCode by Richard Moore
 - TFT_eSPI by Bodmer

Warning must be ignored (display without touch):<br>
#warning >>>>------>> TOUCH_CS pin not defined, TFT_eSPI touch functions will not be available!<br>
<br>

## Main menu for test
![PicoSigner](Files/RPI_PicoSigner_menu.jpg)

## It can read QR code with the wallet words
As debug, It sends the QR code read through serial.<br>
<br>
![PicoSigner](Files/RPI_PicoSigner_QRcode_scan.jpg)


## Camera quality
Camera example with faint colors, but good to read the QR code.<br>
<br>
![PicoSigner](Files/RPI_PicoSigner_like.jpg)


## It can input the wallet words through keyboard
![PicoSigner](Files/RPI_PicoSigner_words.jpg)


## It can input the wallet words typing the letters
![PicoSigner](Files/RPI_PicoSigner_spy.jpg)


## It can generate QR code signing result
![PicoSigner](Files/RPI_PicoSigner_QRcode_out.jpg)

<br>

## Have fun
As it stands, the project can already serve as a reference or as a foundation for someone who wants to improve it.<br>

It still requires additional code before it can be useful for complete Bitcoin transaction signing.<br>


