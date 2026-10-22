# seed_chuck (libDaisy)

[ChucK](https://chuck.stanford.edu) core running on a Daisy Seed with the MA&I pcb board, built with libDaisy.

The ChucK core sources are in [`../chuck`](../chuck) and compiled with `-D__CHUCK_EMBEDDED__`.

The MA&I pcb board controls and LEDs are wrapped by `MplsArtIndustrySeed` in [`../mplsartindustry`](../mplsartindustry).


## Memory

* ChucK is ~1.4 MB, so the program runs from QSPI flash via the Daisy bootloader
  (`APP_TYPE = BOOT_QSPI`).

* `ChucK::init()` allocates several MB, so [`../chuck/sdram_heap.cpp`](../chuck/sdram_heap.cpp) replaces
  `_sbrk`. Heap allocations come from internal SRAM until `EnableSdramHeap()` is called after
  `hw.Init()`, then from a 56 MB SDRAM region.


## Build

Requires the [Daisy toolchain](https://docs.daisy.audio/tutorials/cpp-dev-env/#1-install-the-toolchain) (`arm-none-eabi-gcc`, `dfu-util`).

```bash
# on osx via homebrew
brew install make gcc-arm-embedded dfu-util open-ocd

# once: clone and build libDaisy (ignored by git)
git clone --recurse-submodules https://github.com/electro-smith/libDaisy ../libDaisy
make -C ../libDaisy

make
```

Override the locations with `make LIBDAISY_DIR=... CHUCK_DIR=... MPLSARTINDUSTRY_DIR=...`, and the toolchain with `GCC_PATH=...`.


## Flash

Put the Daisy Seed in DFU mode (hold BOOT, press RESET), then:

```bash
# once: install the Daisy bootloader to internal flash
make program-boot

# within the bootloader's 2 s grace period after reset (LED pulsing)
make program-dfu
```


## Usage

* FS1 (D24) broadcasts the ChucK global event `trigger`, which plays a random note
* POT1 (A1/D16), POT2 (A2/D17), and POT3 (A3/D18) set the envelope attack, decay, and release, 1 to 2000 ms
* SW1 (D10) switches the output between hard and soft clip; LED1 (D22) shows hard clip
* LED2 (D23, DAC OUT 1) brightness shows the note frequency, dim for low notes and bright for high notes
* USB serial prints ChucK console output, SDRAM heap used after compiling, and audio callback CPU load once per second
