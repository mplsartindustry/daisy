/*

    mplsartindustry/daisy
    Copyright (c) 2024-2026 held jointly by the individual authors.

    This file is part of mplsartindustry/daisy.

    mplsartindustry/daisy is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    mplsartindustry/daisy is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with mplsartindustry/daisy.  If not, see <http://www.gnu.org/licenses/>.

*/
#pragma once

#include "daisy_seed.h"

/*

  Daisy Seed pin mapping with MA&I pcb board

  11  D10     --> SW1
  16  in[0]   --> AUDIO_IN_L
  17  in[1]   --> AUDIO_IN_R
  18  out[0]  --> AUDIO_OUT_L
  19  out[1]  --> AUDIO_OUT_R
  23  A1/D16  --> POT1/J7 pin 2
  24  A2/D17  --> POT2/J8 pin 2
  25  A3/D18  --> POT3/J9 pin 2
  29  A7/D22/DAC OUT 2 --> LEDs pin 1
  30  A8/D23/DAC OUT 1 --> LEDs pin 3
  31  A9/D24  --> FS1

 */

namespace mplsartindustry {

/** Potentiometer, an AnalogControl mapped to a range and curve by a Parameter. */
class Potentiometer {
public:
  void Init(uint16_t * adcPtr,
            float updateRate,
            float min = 0.0f,
            float max = 1.0f,
            daisy::Parameter::Curve curve = daisy::Parameter::LINEAR);

  /** Change the mapped range and curve. */
  void SetRange(float min, float max, daisy::Parameter::Curve curve = daisy::Parameter::LINEAR);

  /** Call when the audio callback rate changes. */
  void SetUpdateRate(float updateRate);

  /** Call once per audio callback, returns the mapped value. */
  float Process();

  /** Mapped value from the last Process(). */
  float Value();

private:
  uint16_t * adcPtr;
  float updateRate;
  float min;
  float max;
  daisy::Parameter::Curve curve;
  daisy::Parameter parameter;

  void InitParameter();
};

/** On/off LED on a GPIO pin. */
class DigitalLed {
public:
  void Init(daisy::Pin pin);
  void Set(bool on);
  void Toggle();
  bool On() const;

private:
  daisy::GPIO gpio;
  bool on;
};

/** Dimmable LED on a DAC output, brightness 0.0 is off, (0.0, 1.0] maps to [threshold, 1.0]. */
class AnalogLed {
public:
  /** dac must already be initialized for channel. threshold is the lowest visible level, e.g. 0.55 * 3.3 V = 1.8 V. */
  void Init(daisy::DacHandle * dac, daisy::DacHandle::Channel channel, float threshold = 0.55f);

  void Set(float brightness);
  float Brightness() const;

private:
  daisy::DacHandle * dac;
  daisy::DacHandle::Channel channel;
  float threshold;
  float brightness;
};

/** Latching toggle switch. */
class Toggle {
public:
  void Init(daisy::Pin pin);

  /** Call once per audio callback. */
  void Debounce();

  bool On() const;

  /** Flipped on the last Debounce(), either direction. */
  bool Changed() const;

private:
  daisy::Switch sw;
};

/** Momentary footswitch. */
class Footswitch {
public:
  void Init(daisy::Pin pin);

  /** Call once per audio callback. */
  void Debounce();

  bool Pressed() const;
  bool RisingEdge() const;
  bool FallingEdge() const;
  float TimeHeldMs() const;

private:
  daisy::Switch sw;
};

/** Daisy Seed on the MA&I pcb board. */
class MplsArtIndustrySeed : public daisy::DaisySeed {
public:
  // pin mapping
  static constexpr daisy::Pin SW1 = daisy::seed::D10;
  static constexpr daisy::Pin POT1 = daisy::seed::A1;   // D16
  static constexpr daisy::Pin POT2 = daisy::seed::A2;   // D17
  static constexpr daisy::Pin POT3 = daisy::seed::A3;   // D18
  static constexpr daisy::Pin LED1 = daisy::seed::D22;  // DAC OUT 2, used as GPIO
  static constexpr daisy::Pin LED2 = daisy::seed::D23;  // DAC OUT 1
  static constexpr daisy::Pin FS1 = daisy::seed::D24;

  /** Initializes the Seed, SDRAM, ADC, DAC, controls, and LEDs. */
  void Init(bool boost = true);

  /** Hide DaisySeed versions to keep the pots' update rate in sync. */
  void SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate sampleRate);
  void SetAudioBlockSize(size_t blockSize);

  /** Call at the start of the audio callback, debounces switches and processes pots. */
  void ProcessAllControls();

  /** Clip x to [-1.0, 1.0]. */
  static inline float HardClip(float x) {
    if (x < -1.0f) {
      return -1.0f;
    }
    else if (x > 1.0f) {
      return 1.0f;
    }
    return x;
  }

  /** Soft clip x to [-1.0, 1.0], cubic rational approximation of tanh, 1.0 at and beyond |x| = 3.0. */
  static inline float SoftClip(float x) {
    if (x < -3.0f) {
      return -1.0f;
    }
    else if (x > 3.0f) {
      return 1.0f;
    }
    return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
  }

  Potentiometer pot1;
  Potentiometer pot2;
  Potentiometer pot3;
  Toggle sw1;
  Footswitch fs1;
  DigitalLed led1;
  AnalogLed led2;

private:
  void UpdateControlRates();
};

} // namespace mplsartindustry
