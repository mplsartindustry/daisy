# mplsartindustry (libDaisy)

`MplsArtIndustrySeed` wraps a Daisy Seed on the MA&I pcb board, in [`mplsartindustry_seed.h`](mplsartindustry_seed.h).
It extends `DaisySeed`, so all of the `DaisySeed` API is available, and adds the board's controls and LEDs.

| Member | Class | Pin | Notes |
|---|---|---|---|
| `pot1` | `Potentiometer` | A1/D16 | `Parameter` over `AnalogControl`, range 0.0 to 1.0 until `SetRange()` |
| `pot2` | `Potentiometer` | A2/D17 | |
| `pot3` | `Potentiometer` | A3/D18 | |
| `sw1` | `Toggle` | D10 | `On()`, `Changed()` |
| `fs1` | `Footswitch` | D24 | `Pressed()`, `RisingEdge()`, `FallingEdge()`, `TimeHeldMs()` |
| `led1` | `DigitalLed` | D22 | GPIO, `Set(bool)`, `Toggle()` |
| `led2` | `AnalogLed` | D23, DAC OUT 1 | `Set(float)`, 0.0 is off, (0.0, 1.0] maps to 0.55 to 1.0 of 3.3 V, where the LED is visible |


Static `HardClip(x)` and `SoftClip(x)` clip audio samples to [-1.0, 1.0].


## Usage

```cpp
#include "mplsartindustry_seed.h"

using namespace daisy;
using namespace mplsartindustry;

MplsArtIndustrySeed hw;

void audioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
  hw.ProcessAllControls();

  hw.led1.Set(hw.sw1.On());
  hw.led2.Set(hw.pot1.Value());
  ...
}

int main(void) {
  // initializes the Seed, SDRAM, ADC, DAC, controls, and LEDs
  hw.Init();

  // set the sample rate and block size through hw, which keeps the pots' update rate in sync
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

  hw.pot1.SetRange(1.0f, 2000.0f, Parameter::EXPONENTIAL);

  hw.StartAudio(audioCallback);
  ...
}
```


## Build

Add the directory and source to the sketch Makefile:

```make
MPLSARTINDUSTRY_DIR ?= ../mplsartindustry

CPP_SOURCES = sketch.cpp $(MPLSARTINDUSTRY_DIR)/mplsartindustry_seed.cpp
C_INCLUDES += -I$(MPLSARTINDUSTRY_DIR)
```

See [`../seed_chuck`](../seed_chuck) for an example.
