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
#include "mplsartindustry_seed.h"

using namespace daisy;

namespace mplsartindustry {

//
// Potentiometer
//

void Potentiometer::Init(uint16_t * adcPtr, float updateRate, float min, float max, Parameter::Curve curve) {
  this->adcPtr = adcPtr;
  this->updateRate = updateRate;
  this->min = min;
  this->max = max;
  this->curve = curve;
  InitParameter();
}

void Potentiometer::SetRange(float min, float max, Parameter::Curve curve) {
  this->min = min;
  this->max = max;
  this->curve = curve;
  InitParameter();
}

void Potentiometer::SetUpdateRate(float updateRate) {
  this->updateRate = updateRate;
  InitParameter();
}

float Potentiometer::Process() {
  return parameter.Process();
}

float Potentiometer::Value() {
  return parameter.Value();
}

void Potentiometer::InitParameter() {
  // Parameter keeps its own copy of the AnalogControl, with no way to change it, so recreate both
  AnalogControl control;
  control.Init(adcPtr, updateRate);
  parameter.Init(control, min, max, curve);
}

//
// DigitalLed
//

void DigitalLed::Init(Pin pin) {
  gpio.Init(pin, GPIO::Mode::OUTPUT);
  Set(false);
}

void DigitalLed::Set(bool on) {
  this->on = on;
  gpio.Write(on);
}

void DigitalLed::Toggle() {
  Set(!on);
}

bool DigitalLed::On() const {
  return on;
}

//
// AnalogLed
//

void AnalogLed::Init(DacHandle * dac, DacHandle::Channel channel, float threshold) {
  this->dac = dac;
  this->channel = channel;
  this->threshold = threshold;
  Set(0.0f);
}

void AnalogLed::Set(float brightness) {
  this->brightness = fminf(fmaxf(brightness, 0.0f), 1.0f);
  float level = this->brightness > 0.0f ? threshold + (1.0f - threshold) * this->brightness : 0.0f;
  dac->WriteValue(channel, (uint16_t) (level * 4095.0f));
}

float AnalogLed::Brightness() const {
  return brightness;
}

//
// Toggle
//

void Toggle::Init(Pin pin) {
  sw.Init(pin, 0.0f, Switch::TYPE_TOGGLE, Switch::POLARITY_INVERTED, GPIO::Pull::PULLUP);
}

void Toggle::Debounce() {
  sw.Debounce();
}

bool Toggle::On() const {
  return sw.Pressed();
}

bool Toggle::Changed() const {
  return sw.RisingEdge() || sw.FallingEdge();
}

//
// Footswitch
//

void Footswitch::Init(Pin pin) {
  sw.Init(pin, 0.0f, Switch::TYPE_MOMENTARY, Switch::POLARITY_INVERTED, GPIO::Pull::PULLUP);
}

void Footswitch::Debounce() {
  sw.Debounce();
}

bool Footswitch::Pressed() const {
  return sw.Pressed();
}

bool Footswitch::RisingEdge() const {
  return sw.RisingEdge();
}

bool Footswitch::FallingEdge() const {
  return sw.FallingEdge();
}

float Footswitch::TimeHeldMs() const {
  return sw.TimeHeldMs();
}

//
// MplsArtIndustrySeed
//

void MplsArtIndustrySeed::Init(bool boost) {

  // initialize hardware, including SDRAM
  DaisySeed::Init(boost);

  // pot1, pot2, pot3
  AdcChannelConfig adcConfig[3];
  adcConfig[0].InitSingle(POT1);
  adcConfig[1].InitSingle(POT2);
  adcConfig[2].InitSingle(POT3);
  adc.Init(adcConfig, 3);

  // pots are processed once per audio callback
  pot1.Init(adc.GetPtr(0), AudioCallbackRate());
  pot2.Init(adc.GetPtr(1), AudioCallbackRate());
  pot3.Init(adc.GetPtr(2), AudioCallbackRate());
  adc.Start();

  // led2 on DAC OUT 1; DAC OUT 2 is left to led1 as GPIO
  DacHandle::Config dacConfig;
  dacConfig.chn = DacHandle::Channel::ONE;
  dacConfig.mode = DacHandle::Mode::POLLING;
  dacConfig.bitdepth = DacHandle::BitDepth::BITS_12;
  dacConfig.buff_state = DacHandle::BufferState::ENABLED;
  dac.Init(dacConfig);
  led2.Init(&dac, DacHandle::Channel::ONE);

  led1.Init(LED1);
  sw1.Init(SW1);
  fs1.Init(FS1);
}

void MplsArtIndustrySeed::SetAudioSampleRate(SaiHandle::Config::SampleRate sampleRate) {
  DaisySeed::SetAudioSampleRate(sampleRate);
  UpdateControlRates();
}

void MplsArtIndustrySeed::SetAudioBlockSize(size_t blockSize) {
  DaisySeed::SetAudioBlockSize(blockSize);
  UpdateControlRates();
}

void MplsArtIndustrySeed::ProcessAllControls() {
  sw1.Debounce();
  fs1.Debounce();
  pot1.Process();
  pot2.Process();
  pot3.Process();
}

void MplsArtIndustrySeed::UpdateControlRates() {
  // switches debounce on System::GetNow(), only the pots depend on the callback rate
  pot1.SetUpdateRate(AudioCallbackRate());
  pot2.SetUpdateRate(AudioCallbackRate());
  pot3.SetUpdateRate(AudioCallbackRate());
}

} // namespace mplsartindustry
