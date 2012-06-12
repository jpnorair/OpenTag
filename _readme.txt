About OpenTag:
OpenTag is an open source framework for developing DASH7 solutions.  Here are
some quick facts:
- Can be used to build servers (DASH7 devices) or clients (user interfaces)
- Can be used with "bare metal" MCUs, RTOS MCUs, or POSIX-compliant systems
- Written chiefly in C.  Only C and ASM are permitted in the primary codebase.
- Uses Apache 2.0 license

!!!!!!!! NOTE !!!!!!!!
Code that existed in dev prior to the restructuring is in the Sandbox folder.
Eventually, it will be integrated into the new structure.
!!!!!!!! NOTE !!!!!!!!

Project Directories:
Apps:         Various applications for OpenTag to compile
Board:        Boards supported by OpenTag (not all apps support all boards)
otkernel:     Where the kernel (system.c & friends) lives
OTlib:        Fully platform-independent library code
OTlibext:     Room for future library extensions (e.g. CoAP)
OTplatform:   Platform-bound drivers (i.e. MCU-bound)
OTradio:      RF transceiver-bound drivers
Sandbox:      Code that existed prior to the directory restructuring
Supplements:  Reference materials and support code


Where to get more information:
http://opentag.sourceforge.net
http://dash7.org
http://www.indigresso.com/wiki

--------------------------------------------------------------------------------
Version History:
Changelog for Git repo updates of OpenTag
All times are 24H Pacific Time


17 February 2012, 000 -- Beta 2 snapshot 5
--> 4 snapshots of bug fixes
--> Bug fixes all around on CC430 platform (now working well)
--> Queries are now working, and Demo_Opmode can demo them

- Structural refinements to the system/kernel layer
  - Updated rfevt_frx() to ignore bad packets and go back to RX
  - Updated session task in sys_event_manager() to allow rfevt_frx() to do this
  - Updated sys_event_manager() to contain a Processing task
  - Processing task is a normal, timed task: its runtime affects kernel timing
  - Processing task also manages the re-listen feature by cloning the session
  - Updated rfevt_ftx() to remove Tc clocking now done by sys_event_manager()
  - Updated all sys_goto_...() functions to remove legacy code

- Fixed RIGD slotting
  - Required cleanup of tc, tca, rx_timeout usage
  - Also required that re-listening is handled as a cloned session (see above)
  - Doing re-listening as a cloned session is cleaner/nicer anyway

- Refinement of session module to allow explicit session popping ("scrap")
  - "netstate" element of session data has M2_NETFLAG_SCRAP bit
  - If SCRAP is set, Session task in sys_event_manager() will pop+idle and loop
  
- Fixed a bug that caused beacons to not send the command & dialog templates

- M2QP frame parsing is now more fault-tolerant
  - updated m2qp_parse_frame() to account for device opmode
  - updated m2qp_init() to initialize m2qp.cmd.code with an init-only value
  - updated sub_opgroup_collection() to bypass itself when response disabled
  - updated sub_process_query() to kill re-listening when query overrides it




2 February 2012, 0600 -- Beta 2 snapshot 0
- MASSIVE UPDATE (probably biggest ever update)
- New directory structure: all old code should go into the new structure
- Kernel is nearly bulletproof
- sys_event_manager() is new and improved
- Veelite improvements
- low-power shut-down routines
- Tons of bug fixes all around
- Upper Layers are working seemingly well, and to latest spec
- Fully integrated support for STM32F
- You can't get a MLX73 proto-board, but it is a good reference project
- Too many other improvements to list


3 November 2011, 000 -- Beta 1 snapshot 4
- Yet more small bug fixes
- New Protocol testbed for testing/fixing/verifying Network layer and M2QP
- Therefore, Network Layer and M2QP are fixed


14 October 2011, 000 -- Beta 1 snapshot 3
- Numerous small bug fixes (hard to count)
- Radio driver for CC430 uses full+end interrupts, making it more stable
- State management changes for MPipe driver for CC430
- System Event Manager now runs fully non-blocking, so interrupts can stay off


14 September 2011, 0140 -- Beta 1 snapshot 2
- Improved Ping Pong app that "maybe" works
- Some bug fixes to Mpipe, although ostensibly it is a hack
- System has been tested to a point where it seems to work
- Small changes to protocol module and such things
- Some profiling done on build sizes
  * Everything:      24KB + FS
  * Subcontroller:   20KB + FS
  * Heavy Endpoint:  18KB + FS
  * Medium Endpoint: 16KB + FS
  * Light Endpoint:  13KB + FS


12 September 2011, 0130 -- Beta 1 snapshot 1
- Almost as MASSIVE update to the code as the last, including:
  * More cleaning-up CCS for CC430 builds
  * Updates to M2QP/M2NP layers to improve & align with spec
  * Callbacks in M2QP for requests and response protocol events
  * Reworked Radio Driver that truly runs in the background (non-blocking)
  * Reworked System Module with pre-emptive support, better background
    processing, mutexes.
  * Callbacks in System Module for all sorts of events


3 August 2011, 1200 -- Beta 1 snapshot 0
- It's a snapshot 0, which is kind of like an alpha
- Expect many subsequent updates in the next two weeks
- MASSIVE update to the code, including:
  * Cleaned-up CCS environment for CC430 builds
  * Application Layer protocol support over NDEF _and_ DASH7 datastream
  * More mature (and somewhat tested) C API
  * Message-based ALP API, for wireline command & diagnostic, based on C API
  * MPipe serial interface & command protocol
  * Some restructuring of the Queues
  * Some clean-up of Veelite for improved performance and smaller size
- Things to-do for snapshot 1
  * Complete testing of MPipe
  * Complete testing of M2NP/M2QP
  * Fix some lingering radio bugs


24 April 2011, 1120 -- Beta 0 snapshot 2
- Fixed a bug in the hold scan list cycler
- Fixed a bug in TI's CC430 radio HAL code: the flush-buffer strobe could not be
  called in sleep, now it can.
- Fixed a few other little things that may have lead to instability when config
  parameters are ill-defined.
- Added platform-wide support for blinking the red and green LEDs on the CC430
  EM board.
- Resolved the issue relating to unpredictable CCA.  CC430 HW CCA mechanism is
  not reliable.  Using the RSSI_Valid interrupt and manually comparing RSSI
  works better (and actually seems to be faster, go figure).
- Changed the beacon nextevent decrementer, which makes activation more
  periodic.


18 April 2011, 2300 -- Beta 0 snapshot 1
- Fixed a veelite bug (not sure how I missed this before)
- General bug fixes for CC430 Radio Module (order of operations stuff)
- Debugging system event handler (needs more debuggers)
- Working on full-system testbed app for CC430
- NOTE: I'm having issues with the device sometimes not waking from sleep.  This
  appears to be an issue with the TI FET debugger box and CCS, which I hate to
  say aren't nearly as good as JLink + IAR.


12 April 2011, 0814 -- Beta 0 snapshot 0
- Working RF test for CC430
- Working POSIX simulation for most testbeds
- Note: POSIX system testbed is in flux.  Getting this to work right requires a
  lot of real-time fiddling in POSIX, which is difficult.  Strangely, it is
  easier to work with it on the MCU.
