# About HT XR Test (ht_xrttest)
HT XR Test is useful for range testing purposes of XR packets.  All it does is broadcast XR packets indefinitely.

**Radio parameters**

* LoRa @ 915 MHz
* 1.3 kbps: 41.7 kHz bandwidth, Speading Factor 7, Coding 4/7
* 20 dBm

## Special Note on Antenna Selection

*Some dev kits have integrated debuggers, voltage converters, or other circuitry that is noisy enough to have a dramatic effect on the range/sensitivity of the radio.*

*The STMicro Lora Discovery board (the Blue ST board with the metal-can module on it), has noise in the -120dBm range, which WILL markedly affect sensitivity.*

*For the STMicro LoRa Discovery board, we recommend using an external antenna.  60 cm (two feet) of separation is enough to get away from the board noise.  In our internal testing, this makes the difference between getting hits at 4 miles and getting hits at 20 miles in open space, or 0.25 miles -> 0.7 miles in ground-to-ground urban/suburban.*

## Console Output

HT XR test will print to the console when a message is received successfully.  It will include the RSSI and calculated linkloss.  

On boards with a Blue LED, XR Test will illuminate the Blue LED during times when the XR packets are being received.  If reception fails, the LED will go off.


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