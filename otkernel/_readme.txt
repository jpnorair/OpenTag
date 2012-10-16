OpenTag Kernels
===============
OpenTag can be used with different kernels, as long as they implement the
interface described in otlib/system.h.  Of course, there are some deeply 
embedded tasks (namely DLL) that may need to be tweaked, but basically the
interface can be universal.

Kernels Available
=================
There are currently two kernels available.  In the future, there as likely to
be more.  You should choose a kernel based on the application requirements as
well as the capabilities of your platform.

  * GULP: "Global [interrupt] Ultra Low Power"
  * HICCULP: "Hardware Integrated Context Caching Ultra Low Power"
  * Future Kernel (maybe derived from ChibiOS)


GULP
----
GULP is the standard kernel for MSP430 and CC430 devices.  If OpenTag is ever
ported to the likes of AVR or PIC devices, GULP will be for them as well.

The GULP kernel means: Global [interrupt] Ultra Low Power.  It is a good kernel
to use for devices with Global interrupt only (e.g. MSP430).  It is also fast,
lightweight, and emphasizes low power over high performance.  That is, it has a
good scheduler for optimizing latency of critical & power hungry I/O tasks, but
it is not the best at maximizing CPU utilization.

GULP uses a hybrid kernel design.  Parent tasks run in co-operative mode, and
are context-free.  Parent tasks may spawn pre-emptive child tasks, which are
typically interrupt-driven and for I/O (e.g. DASH7 RF, MPipe).  Parent tasks
must manage the child tasks, and shut them down if they have problems.

GULP is also event-based.  It does not use a system tick, although it is fully
re-entrant in case you would like to implement one.  When the GULP scheduler
runs, it will pick the active task with highest priority and most urgent status, 
and it will put the system to sleep until this task needs to be invoked.

GULP has means for specifying the latency and runtime requirements of a task.
These values are dynamic, and they can be altered depending on the state of the
task.  An example of "latency" is the request-response turnaround of a protocol
task.  I can set it to a low value, which will *guarantee* that the turnaround
is at least that fast, that no lower-priority task will be hogging the CPU when
this task needs the CPU.

GULP tasks and their priorities must be defined at compile time.  Since GULP is
intended for small microcontrollers, this is not viewed as a problem.  It is
possible, however, to allocate a task at compile time and dynamically bind it
to different task functions.  This is a compile-time option.  To save resources
it is also possible to make all task functions static.

Writing co-operative tasks for GULP is not difficult, but there is a small 
learning curve.  The developer must manage the state of the task and set 
pre-emption points if the task needs to run for a long time.  Writing fully
pre-emptive child tasks is not as simple or forgiving with GULP, but as long as
the I/O driver is functioning properly, it is something that an experienced
embedded developer can pick up quickly.

GULP's attributes make it a good choice for endpoints & subcontrollers.  It is 
OK for simple gateways, too, for example a gateway with just an MPipe interface 
and some simple control logic. If you are using a device with a more 
sophisticated interrupt controller, such as a Cortex M3, you can use the 
HICCULP kernel to get virtually all the same benefits as GULP, but better 
support for intensive multi-tasking.


HICCULP
-------
HICCULP is the standard kernel for Cortex M3 devices.  It is very similar to
GULP except that it is designed specifically to leverage the NVIC hardware in
the Cortex M3 (and perhaps also the Cortex M0).  

The GULP kernel has difficulty running parallel, low-latency I/O tasks.  The
HICCULP kernel, generally, does not.  Additionally, the HICCULP kernel provides
a means to bind any task to a fully pre-emptive context.  The caveat is that
the Cortex M3 has some limitations for HW context switching.  It is possible to
have as many as SEVEN fully pre-emptive, prioritized tasks on HICCULP.  The 
kernel and the co-operative tasks occupy one of the context slots, as the HW
itself supports up to eight.

One benefit of the HICCULP kernel is that it is somewhat more fault-tolerant in
regard to design and authoring of pre-emptive tasks than GULP is.  If you 
intend to deploy on GULP and HICCULP, it is better to design on GULP and then
port to HICCULP.

HICCULP is preferred for running OpenTag along with one or two additional
high-demand protocol stacks.  USB is an example, as is Ethernet/IP.  HICCULP is 
also the natural choice for integrating OpenTag with an environment such as 
Maple, as you can give Maple its own context in which to run. 
(Maple: http://www.leaflabs.com)

