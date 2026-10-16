# patch_init_chuck (libDaisy)

[ChucK](https://chuck.stanford.edu) core running on a Daisy Patch SM (patch.Init()), built with libDaisy.

The ChucK core sources are in [`../chuck`](../chuck) and compiled with `-D__CHUCK_EMBEDDED__`.


## Memory

* ChucK is ~1.4 MB, so the program runs from QSPI flash via the Daisy bootloader
  (`APP_TYPE = BOOT_QSPI`).

* `ChucK::init()` allocates several MB, so [`../chuck/sdram_heap.cpp`](../chuck/sdram_heap.cpp) replaces
  `_sbrk`. Heap allocations come from internal SRAM until `EnableSdramHeap()` is called after
  `patch.Init()`, then from a 56 MB SDRAM region.


## Build

Requires the [Daisy toolchain](https://docs.daisy.audio/tutorials/cpp-dev-env/#1-install-the-toolchain) (`arm-none-eabi-gcc`, `dfu-util`).

Override the locations with `make LIBDAISY_DIR=... CHUCK_DIR=...`, and the toolchain with `GCC_PATH=...`, if necessary.

```bash
# on osx via homebrew

$ brew install make gcc-arm-embedded dfu-util open-ocd


# once: clone and build libDaisy (ignored by git)

$ git clone --recurse-submodules https://github.com/electro-smith/libDaisy ../libDaisy
$ make -C ../libDaisy
...
arm-none-eabi-ar: creating build/libdaisy.a

# build sketch to .hex and .bin files

$ make
...
arm-none-eabi-objcopy -O ihex build/patch_init_chuck.elf build/patch_init_chuck.hex
arm-none-eabi-objcopy -O binary -S build/patch_init_chuck.elf build/patch_init_chuck.bin
```


## Flash

Put the Patch SM in DFU mode (hold BOOT, press RESET), then:

```bash
#
# once: install the Daisy bootloader to internal flash
#

$ make program-boot
dfu-util -a 0 -s 0x08000000:leave -D ../libDaisy/core/dsy_bootloader_v6_4-intdfu-2000ms.bin -d ,0483:df11
dfu-util 0.11

Copyright 2005-2009 Weston Schmidt, Harald Welte and OpenMoko Inc.
Copyright 2010-2021 Tormod Volden and Stefan Schmidt
This program is Free Software and has ABSOLUTELY NO WARRANTY
Please report bugs to http://sourceforge.net/p/dfu-util/tickets/

dfu-util: Warning: Invalid DFU suffix signature
dfu-util: A valid DFU suffix will be required in a future dfu-util release
Opening DFU capable USB device...
Device ID 0483:df11
Device DFU version 011a
Claiming USB DFU Interface...
Setting Alternate Interface #0 ...
Determining device status...
DFU state(2) = dfuIDLE, status(0) = No error condition is present
DFU mode device DFU version 011a
Device returned transfer size 1024
DfuSe interface name: "Internal Flash   "
Downloading element to address = 0x08000000, size = 130188
Erase   	[=========================] 100%       130188 bytes
Erase    done.
Download	[=========================] 100%       130188 bytes
Download done.
File downloaded successfully
Submitting leave request...
dfu-util: Error during download get_status
make: *** [program-boot] Error 74

#
# error at the end most likely ignorable!


#
# then: within the bootloader's 2 s grace period after reset (LED pulsing)
#

$ make program-dfu
dfu-util -a 0 -s 0x90040000:leave -D build/patch_init_chuck.bin -d ,0483:df11
dfu-util 0.11

Copyright 2005-2009 Weston Schmidt, Harald Welte and OpenMoko Inc.
Copyright 2010-2021 Tormod Volden and Stefan Schmidt
This program is Free Software and has ABSOLUTELY NO WARRANTY
Please report bugs to http://sourceforge.net/p/dfu-util/tickets/

dfu-util: Warning: Invalid DFU suffix signature
dfu-util: A valid DFU suffix will be required in a future dfu-util release
Opening DFU capable USB device...
Device ID 0483:df11
Device DFU version 011a
Claiming USB DFU Interface...
Setting Alternate Interface #0 ...
Determining device status...
DFU state(2) = dfuIDLE, status(0) = No error condition is present
DFU mode device DFU version 011a
Device returned transfer size 4096
DfuSe interface name: "Flash "
Downloading element to address = 0x90040000, size = 1468556
Erase   	[=========================] 100%      1468556 bytes
Erase    done.
Download	[=========================] 100%      1468556 bytes
Download done.
File downloaded successfully
Submitting leave request...
Transitioning to dfuMANIFEST state
```


### USB serial

The Daisy shows up as a USB serial device, but a serial port isn't a regular file, so tail doesn't work well on it. Open it with a terminal program instead.

1. Find the port (after the app starts, once the DFU device disappears):

   ```bash
   ls /dev/cu.usbmodem*
   ```

   macOS creates two entries for each port, `/dev/tty.usbmodem…` and `/dev/cu.usbmodem…`. Use `cu.`, because `tty.` can hang when you open it.

2. Open the port:

   ```bash
   screen /dev/cu.usbmodem<NNNN> 115200
   ```

   The baud rate doesn't matter for USB serial, so any value works. To quit `screen`, press `Ctrl-A`, then `K`, then `y`. Other options:

   * `cat /dev/cu.usbmodem<NNNN>` is the closest to `tail -f`. Quit with `Ctrl-C`.
   * `arduino-cli monitor -p /dev/cu.usbmodem<NNNN>` works if you have arduino-cli installed.
   * The Arduino IDE's Serial Monitor also works.

3. If no `usbmodem` appears, check whether the board shows up at all:

   ```bash
   system_profiler SPUSBDataType | grep -i -A6 daisy
   # or
   ioreg -p IOUSB -l | grep -i -B2 -A10 daisy
   ```

   What you see narrows things down:

   * Only the DFU device appears (`0483:df11`, "DFU in FS Mode"): you're still in the bootloader, and the app hasn't started.
   * Nothing Daisy-related appears: the app is running but USB never connected. The audio callback may be using all the CPU so USB never starts.
   * A Daisy/STM device appears, but there's no `cu.usbmodem` entry: the USB serial part isn't working.

Remember that with `StartLog(false)`, anything printed before you connect is lost. If the main loop is running, you should still see `CPU load avg: … max: …` once per second.


## Usage

* B7 broadcasts the ChucK global event `trigger`, which plays a random note
* B8 switches the output between hard and soft clip; the C1 LED shows hard clip
* USB serial prints ChucK console output, SDRAM heap used after compiling, and audio callback CPU load once per second
