# patch_init_chuck (libDaisy)

[ChucK](https://chuck.stanford.edu) core running on a Daisy Patch SM (patch.Init()), built with libDaisy.

The ChucK core sources are in [`../chuck`](../chuck), shared with
[`../seed_chuck`](../seed_chuck), and compiled with `-D__CHUCK_EMBEDDED__`.

## Memory

* ChucK is ~1.4 MB, so the program runs from QSPI flash via the Daisy bootloader
  (`APP_TYPE = BOOT_QSPI`).
* `ChucK::init()` allocates several MB, so [`../chuck/sdram_heap.cpp`](../chuck/sdram_heap.cpp) replaces `_sbrk`. Heap
  allocations come from internal SRAM until `EnableSdramHeap()` is called after
  `patch.Init()`, then from a 56 MB SDRAM region.

## Build

Requires the [Daisy toolchain](https://daisy.audio/tutorials/cpp-dev-env/) (`arm-none-eabi-gcc`, `dfu-util`).

```bash
# once: clone and build libDaisy (ignored by git)
git clone --recurse-submodules https://github.com/electro-smith/libDaisy ../libDaisy
make -C ../libDaisy

make
```

Override the locations with `make LIBDAISY_DIR=... CHUCK_DIR=...`, and the toolchain with `GCC_PATH=...`.

## Flash

Put the Patch SM in DFU mode (hold BOOT, press RESET), then:

```bash
# once: install the Daisy bootloader to internal flash
make program-boot

# within the bootloader's 2 s grace period after reset (LED pulsing)
make program-dfu
```

## Usage

* B7 broadcasts the ChucK global event `trigger`, which plays a random note
* B8 switches the output between hard and soft clip; the C1 LED shows hard clip
* USB serial prints ChucK console output, SDRAM heap used after compiling, and audio callback CPU load once per second
