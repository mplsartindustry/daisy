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
#include "util/CpuLoadMeter.h"
#include "chuck.h"
#include "chuck_globals.h"
#include "sdram_heap.h"

using namespace daisy;
using namespace mplsartindustry;

MplsArtIndustrySeed hw;

ChucK * chuck = NULL;
SAMPLE * inputBuffer = NULL;
SAMPLE * outputBuffer = NULL;
t_CKFLOAT * freq = NULL;

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

  hw.ProcessAllControls();

  bool rising = hw.fs1.RisingEdge();
  bool hard = hw.sw1.On();

  if (rising) {
    // set envelope before trigger, ChucK handles requests in order
    chuck->globals()->setGlobalFloat("attack", hw.pot1.Value());
    chuck->globals()->setGlobalFloat("decay", hw.pot2.Value());
    chuck->globals()->setGlobalFloat("release", hw.pot3.Value());
    chuck->globals()->broadcastGlobalEvent("trigger");
  }

  // led1 shows hard clip
  hw.led1.Set(hard);

  // interleave input buffers
  for (size_t i = 0; i < size; i++) {
    inputBuffer[2 * i] = in[0][i];
    inputBuffer[2 * i + 1] = in[1][i];
  }

  // run ChucK for the next `size` frames
  chuck->run(inputBuffer, outputBuffer, size);

  // led2 brightness shows note frequency
  hw.led2.Set(ledBrightness(*freq));

  // deinterleave and clip output buffers
  for (size_t i = 0; i < size; i++) {
    out[0][i] = hard ? hw.HardClip(outputBuffer[2 * i]) : hw.SoftClip(outputBuffer[2 * i]);
    out[1][i] = hard ? hw.HardClip(outputBuffer[2 * i + 1]) : hw.SoftClip(outputBuffer[2 * i + 1]);
  }

  cpuLoad.OnBlockEnd();
}

void chuckPrint(const char * message) {
  DaisySeed::Print("%s", message);
}

int main(void) {

  // initialize hardware, including SDRAM, controls, and LEDs
  hw.Init();
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);

  // ChucK allocates several MB, more than internal SRAM
  EnableSdramHeap();

  // pot1, pot2, pot3, envelope attack, decay, release, 1 to 2000 ms, finer at the low end
  hw.pot1.SetRange(1.0f, 2000.0f, Parameter::EXPONENTIAL);
  hw.pot2.SetRange(1.0f, 2000.0f, Parameter::EXPONENTIAL);
  hw.pot3.SetRange(1.0f, 2000.0f, Parameter::EXPONENTIAL);

  // sw1, toggle switch, hard clip/soft clip, shown on led1
  // fs1, footswitch, trigger
  // led2, note frequency

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

// map note frequency, MIDI notes 48 to 72, to LED brightness 0.0 to 1.0; 0.0 (off) before the first note
// AnalogLed treats 0.0 as off, so the lowest note maps just above 0.0 to stay lit
float ledBrightness(float freq) {
  if (freq <= 0.0f) {
    return 0.0f;
  }
  float note = 69.0f + 12.0f * log2f(freq / 440.0f);
  return fminf(fmaxf((note - 48.0f) / 24.0f, 0.001f), 1.0f);
}
