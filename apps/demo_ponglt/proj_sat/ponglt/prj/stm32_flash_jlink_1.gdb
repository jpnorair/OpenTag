# connect to the J-Link gdb server
target remote localhost:2331

# Enable flash download and flash breakpoints.
# Flash download and flash breakpoints are features of
# the J-Link software which require separate licenses 
# from SEGGER.

# Select flash device
monitor flash device = STM32F103RB

# Enable FlashDL and FlashBPs
monitor flash download = 1
monitor flash breakpoints = 1

# Clear all pendig breakpoints
monitor clrbp

# Set gdb server to little endian
monitor endian little

# Set JTAG speed to 5 kHz
monitor speed 5

# Reset the target
monitor reset
monitor sleep 100

# Set JTAG speed in khz
monitor speed auto

# Vector table placed in Flash
monitor writeu32 0xE000ED08 = 0x00000000
