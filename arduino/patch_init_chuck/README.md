# patch_init_chuck

[ChucK](https://chuck.stanford.edu) core running inside a Daisy Patch SM (patch.Init()) Arduino sketch.

## ChucK core

The `*.c`, `*.cpp`, and `*.h` files in this directory are copied from
`chuck/src/core` (commit `3eaa05ed`), with these exceptions:

* not copied: `lo/` (liblo), `util_network.*`, `util_opsc.*`, `ulib_opsc.*`,
  `rtmidi.*`, `midiio_rtmidi.*`, `hidio_sdl.*`, `util_hid.*`, `util_serial.*`,
  `chuck_otf.*`, `chuck_shell.*`, `util_sndfile.*`, `dirent_win32.h`, `chuck_yacc.*`
* `chuck_def.h` defines `__PLATFORM_EMBEDDED__` when `ARDUINO` (or
  `__CHUCK_EMBEDDED__`) is defined. This turns on the existing
  `__DISABLE_*__` switches (network, OTF server, shell, threads, MIDI, HID,
  serial, file IO, SndBuf, WvOut, kbhit) and adds `__DISABLE_FILESYSTEM__`,
  `__DISABLE_CHUGINS__`, and `__DISABLE_RTTI__`.
* the source files are patched for `__PLATFORM_EMBEDDED__` and those switches:
  no `dlopen`, `opendir`, `realpath`, `glob`/`wordexp`, `getcwd`, `ioctl`,
  `pthread`, `random_device`, `typeid`, or `throw`. There are also weak `_open`/`_stat`
  stubs, so `fopen()` and `stat()` fail at runtime.
* the STK raw wavetables in `util_raw.c` are `const` (flash, not RAM), and the
  symbol hash table is smaller on embedded.

To build the same configuration on a desktop, compile with `-D__CHUCK_EMBEDDED__`.

## Memory

ChucK is much larger than the usual Daisy sketch:

* code + read-only data is about 1.2 MB, but the STM32H750 internal flash is 128 KB.
  The sketch compiles, but it does not fit at link time with the stock DaisyDuino
  board definition. It has to run from QSPI flash (Daisy bootloader).
* `ChucK::init()` allocates a few MB of heap (about 4.7 MB measured on 64-bit
  macOS). That is more than the 512 KB of AXI SRAM, so the heap has to be in
  SDRAM.

[`../../libdaisy/patch_init_chuck`](../../libdaisy/patch_init_chuck) builds the same
ChucK sources with libDaisy, which handles both.
