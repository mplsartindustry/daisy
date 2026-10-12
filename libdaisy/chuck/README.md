# chuck

[ChucK](https://chuck.stanford.edu) core, shared by the libDaisy projects.


## Differences from ChucK core

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


Projects within this directory compile them with `-D__CHUCK_EMBEDDED__` by
setting `CHUCK_DIR` in their Makefile.

Please note `sdram_heap.cpp`/`.h` are not part of ChucK. They replace `_sbrk`
so that the heap moves to SDRAM once `EnableSdramHeap()` is called after hardware
`Init()`, since `ChucK::init()` allocates several MB, more than internal SRAM.
