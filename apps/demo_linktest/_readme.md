# About HT Linktest (ht_linktest)
HT Linktest is a simple, OpenTag demo application for master-slave link testing.  It is not a low-power demo, as the devices are always listening or transmitting.


## Functional purpose of this demo
 - Testing the link.  It sends 100 packets (by default) so it will compute the PER across 100 packets.
 - Serve similar API and programming model purposes as API and Ping Pong Demos.


## Application Description
HT Linktest sends 100 beacons from one device to another.  The beaconing devices is called the Endpoint and the listening device is called the  Gateway.  If you hold-down the application button on startup, this tells the device to be the Endpoint.

* A button press on the Endpoint tells it to send 100 (default) beacons.


## Stock Applets Used
Many of the stock applets from /otlibext/stdapplets/ are used as signal 
callbacks from the protocol layers.  You can look at extf_config.h to see which ones are enabled (or, to specify new ones).


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