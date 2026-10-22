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
#include "daisy_seed.h"
#include "util/CpuLoadMeter.h"
#include "chuck.h"
#include "chuck_globals.h"
#include "sdram_heap.h"

using namespace daisy;

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

DaisySeed hw;
Switch sw1;
Switch fs1;
GPIO led1;
AnalogControl pot1;
AnalogControl pot2;
AnalogControl pot3;

ChucK * chuck = NULL;
SAMPLE * inputBuffer = NULL;
SAMPLE * outputBuffer = NULL;
t_CKFLOAT * freq = NULL;

float hardClip(float x);
float softClip(float x);
float envelopeMs(float x);
float ledBrightness(float freq);

CpuLoadMeter cpuLoad;

// chuck code
const char * CHUCK_CODE = R"(
global Event trigger;
global float attack;
global float decay;
global float release;
global float freq;

SinOsc s => ADSR e => dac;
220 => s.freq;
0.5 => s.gain;

while (true) {
  trigger => now;
  e.set(attack::ms, decay::ms, 0.5, release::ms);
  Math.random2(48, 72) => Std.mtof => freq => s.freq;
  e.keyOn();
  (attack + decay)::ms => now;
  e.keyOff();
}
)";

void audioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {

  cpuLoad.OnBlockStart();

  sw1.Debounce();
  fs1.Debounce();

  pot1.Process();
  pot2.Process();
  pot3.Process();

  bool rising = fs1.RisingEdge();
  bool hard = sw1.Pressed();

  if (rising) {
    // set envelope before trigger, ChucK handles requests in order
    chuck->globals()->setGlobalFloat("attack", envelopeMs(pot1.Value()));
    chuck->globals()->setGlobalFloat("decay", envelopeMs(pot2.Value()));
    chuck->globals()->setGlobalFloat("release", envelopeMs(pot3.Value()));
    chuck->globals()->broadcastGlobalEvent("trigger");
  }

  // led1 shows hard clip
  led1.Write(hard);

  // interleave input buffers
  for (size_t i = 0; i < size; i++) {
    inputBuffer[2 * i] = in[0][i];
    inputBuffer[2 * i + 1] = in[1][i];
  }

  // run ChucK for the next `size` frames
  chuck->run(inputBuffer, outputBuffer, size);

  // led2 brightness shows note frequency
  hw.dac.WriteValue(DacHandle::Channel::ONE, (uint16_t) (ledBrightness(*freq) * 4095.0f));

  // deinterleave and clip output buffers
  for (size_t i = 0; i < size; i++) {
    out[0][i] = hard ? hardClip(outputBuffer[2 * i]) : softClip(outputBuffer[2 * i]);
    out[1][i] = hard ? hardClip(outputBuffer[2 * i + 1]) : softClip(outputBuffer[2 * i + 1]);
  }

  cpuLoad.OnBlockEnd();
}

void chuckPrint(const char * message) {
  DaisySeed::Print("%s", message);
}

int main(void) {

  // initialize hardware, including SDRAM
  hw.Init(true);
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);

  // ChucK allocates several MB, more than internal SRAM
  EnableSdramHeap();

  // sw1, toggle switch, hard clip/soft clip
  sw1.Init(seed::D10);

  // fs1, footswitch, trigger
  fs1.Init(seed::D24);

  // led1, hard clip
  led1.Init(seed::D22, GPIO::Mode::OUTPUT);

  // pot1, pot2, pot3, potentiometers, envelope attack, decay, release
  AdcChannelConfig adcConfig[3];
  adcConfig[0].InitSingle(seed::A1);
  adcConfig[1].InitSingle(seed::A2);
  adcConfig[2].InitSingle(seed::A3);
  hw.adc.Init(adcConfig, 3);

  // controls are processed once per audio callback
  pot1.Init(hw.adc.GetPtr(0), hw.AudioCallbackRate());
  pot2.Init(hw.adc.GetPtr(1), hw.AudioCallbackRate());
  pot3.Init(hw.adc.GetPtr(2), hw.AudioCallbackRate());
  hw.adc.Start();

  // led2, DAC OUT 1, note frequency
  DacHandle::Config dacConfig;
  dacConfig.chn = DacHandle::Channel::ONE;
  dacConfig.mode = DacHandle::Mode::POLLING;
  dacConfig.bitdepth = DacHandle::BitDepth::BITS_12;
  dacConfig.buff_state = DacHandle::BufferState::ENABLED;
  hw.dac.Init(dacConfig);
  hw.dac.WriteValue(DacHandle::Channel::ONE, 0);

  // ChucK console output over USB serial
  DaisySeed::StartLog(false);
  ChucK::setStdoutCallback(chuckPrint);
  ChucK::setStderrCallback(chuckPrint);

  // initialize ChucK
  chuck = new ChucK();

  chuck->setParam(CHUCK_PARAM_SAMPLE_RATE, (t_CKINT) hw.AudioSampleRate());
  chuck->setParam(CHUCK_PARAM_INPUT_CHANNELS, 2);
  chuck->setParam(CHUCK_PARAM_OUTPUT_CHANNELS, 2);
  chuck->setParam(CHUCK_PARAM_VM_HALT, FALSE);
  chuck->setParam(CHUCK_PARAM_IS_REALTIME_AUDIO_HINT, TRUE);
  chuck->setParam(CHUCK_PARAM_CHUGIN_ENABLE, FALSE);

  // allocate audio buffers, after parameters are set
  size_t bufferSize = hw.AudioBlockSize();
  inputBuffer = new SAMPLE[bufferSize * 2]();
  outputBuffer = new SAMPLE[bufferSize * 2]();

  chuck->init();
  chuck->start();

  if (!chuck->compileCode(CHUCK_CODE, "", 1)) {
    DaisySeed::PrintLine("ChucK compile failed");
  }
  DaisySeed::PrintLine("ChucK heap: %u bytes", (unsigned) SdramHeapUsed());

  // read global freq directly from the VM in the audio callback, the same thread as run()
  chuck->globals()->init_global_float("freq");
  freq = chuck->globals()->get_ptr_to_global_float("freq");

  cpuLoad.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
  hw.StartAudio(audioCallback);

  while (1) {
    // report audio callback CPU load once per second
    System::Delay(1000);
    DaisySeed::PrintLine("CPU load avg: " FLT_FMT3 " max: " FLT_FMT3,
                            FLT_VAR3(cpuLoad.GetAvgCpuLoad()),
                            FLT_VAR3(cpuLoad.GetMaxCpuLoad()));
  }
}

float hardClip(float x) {
  if (x < -1.0f) {
    return -1.0f;
  }
  else if (x > 1.0f) {
    return 1.0f;
  }
  return x;
}

float softClip(float x) {
  if (x < -3.0f) {
    return -1.0f;
  }
  else if (x > 3.0f) {
    return 1.0f;
  }
  return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
}

// map pot value 0.0 to 1.0 to envelope time 1 to 2000 ms, finer at the low end
float envelopeMs(float x) {
  return 1.0f + 1999.0f * x * x;
}

// map note frequency, MIDI notes 48 to 72, to LED brightness 0.55 to 1.0; 0.0 before the first note
// LEDs do not light below ~1.8 V, so the lowest note starts at 0.55 * 3.3 V = 1.8 V
float ledBrightness(float freq) {
  if (freq <= 0.0f) {
    return 0.0f;
  }
  float note = 69.0f + 12.0f * log2f(freq / 440.0f);
  float x = fminf(fmaxf((note - 48.0f) / 24.0f, 0.0f), 1.0f);
  return 0.55f + 0.45f * x;
}
