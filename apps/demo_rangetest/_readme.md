# About HT Rangetest (ht_rangetest)
HT Rangetest is useful for range testing purposes.  All it does is broadcast beacons indefinitely.

**Radio parameters**

* LoRa @ 915 MHz
* 15.6 kbps: 500 kHz bandwidth, Speading Factor 7, Coding 4/7
* 20 dBm

## Console Output

HT Rangetest will print to the console when a message is received successfully.  It will include the RSSI and calculated linkloss.  

It will also print the contents of packets that were not correctly received.  Sometimes these are false-positives due to noise on the channel (915 MHz is an ISM band, there are lots of users).  Sometimes it is due to receive a range test beacon that had some uncorrectable errors in it.


## This Application Recommends

* MCU Requirements
 - All supported configurations should be fine.
* Feature/Build Requirements
 - MPipe connection to show link test data
 - Two LEDs to show RX/TX activity
 - One input source to send pings (a button), or sending by ALP over MPipe.


## Supported Boards & Platforms
See /apps/demo_ponglt/app/board_config.h for board & platform support.
NOTE: Different boards may support different methods of input and output.


## Supported Project IDEs
For this demo app, if you are using STM32 then AC6 System Workbench Toolchain is what you should use.


## Wiki Link & More Information
(no link)


--------------------------------------------------------------------------------
## Usage Notes
(no notes)