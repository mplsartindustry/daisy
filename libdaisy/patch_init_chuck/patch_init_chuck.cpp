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
#include "daisy_patch_sm.h"
#include "util/CpuLoadMeter.h"
#include "chuck.h"
#include "chuck_globals.h"
#include "sdram_heap.h"

using namespace daisy;
using namespace daisy::patch_sm;

DaisyPatchSM patch;
Switch b7;
Switch b8;
CpuLoadMeter cpuLoad;

// ChucK instance
ChucK * chuck = NULL;

// interleaved audio buffers, as expected by ChucK::run()
SAMPLE * inputBuffer = NULL;
SAMPLE * outputBuffer = NULL;

// ChucK program, b7 broadcasts global event `trigger`
const char * CHUCK_CODE = R"(
global Event trigger;

SinOsc s => ADSR e => dac;
220 => s.freq;
0.5 => s.gain;
e.set(5::ms, 100::ms, 0.0, 10::ms);

while (true) {
  trigger => now;
  Math.random2(48, 72) => Std.mtof => s.freq;
  e.keyOn();
}
)";

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

void audioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {

  cpuLoad.OnBlockStart();

  patch.ProcessAllControls();

  b7.Debounce();
  b8.Debounce();

  bool rising = b7.RisingEdge();
  bool hard = b8.Pressed();

  if (rising) {
    chuck->globals()->broadcastGlobalEvent("trigger");
  }

  // cv_out_2, C1 led on front panel
  patch.WriteCvOut(CV_OUT_2, hard ? 5.0f : 0.0f);

  // interleave input buffers
  for (size_t i = 0; i < size; i++) {
    inputBuffer[2 * i] = in[0][i];
    inputBuffer[2 * i + 1] = in[1][i];
  }

  // run ChucK for the next `size` frames
  chuck->run(inputBuffer, outputBuffer, size);

  // deinterleave and clip output buffers
  for (size_t i = 0; i < size; i++) {
    out[0][i] = hard ? hardClip(outputBuffer[2 * i]) : softClip(outputBuffer[2 * i]);
    out[1][i] = hard ? hardClip(outputBuffer[2 * i + 1]) : softClip(outputBuffer[2 * i + 1]);
  }

  cpuLoad.OnBlockEnd();
}

void chuckPrint(const char * message) {
  DaisyPatchSM::Print("%s", message);
}

int main(void) {

  // initialize hardware, including SDRAM
  patch.Init();
  patch.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);

  // ChucK allocates several MB, more than internal SRAM
  EnableSdramHeap();

  // b7, momentary button B7, trigger
  b7.Init(DaisyPatchSM::B7);

  // b8, toggle switch, hard clip/soft clip
  b8.Init(DaisyPatchSM::B8);

  // ChucK console output over USB serial
  DaisyPatchSM::StartLog(false);
  ChucK::setStdoutCallback(chuckPrint);
  ChucK::setStderrCallback(chuckPrint);

  // initialize ChucK
  chuck = new ChucK();

  chuck->setParam(CHUCK_PARAM_SAMPLE_RATE, (t_CKINT) patch.AudioSampleRate());
  chuck->setParam(CHUCK_PARAM_INPUT_CHANNELS, 2);
  chuck->setParam(CHUCK_PARAM_OUTPUT_CHANNELS, 2);
  chuck->setParam(CHUCK_PARAM_VM_HALT, FALSE);
  chuck->setParam(CHUCK_PARAM_IS_REALTIME_AUDIO_HINT, TRUE);
  chuck->setParam(CHUCK_PARAM_CHUGIN_ENABLE, FALSE);

  // allocate audio buffers, after parameters are set
  size_t bufferSize = patch.AudioBlockSize();
  inputBuffer = new SAMPLE[bufferSize * 2]();
  outputBuffer = new SAMPLE[bufferSize * 2]();

  chuck->init();
  chuck->start();

  if (!chuck->compileCode(CHUCK_CODE, "", 1)) {
    DaisyPatchSM::PrintLine("ChucK compile failed");
  }
  DaisyPatchSM::PrintLine("ChucK heap: %u bytes", (unsigned) SdramHeapUsed());

  cpuLoad.Init(patch.AudioSampleRate(), patch.AudioBlockSize());
  patch.StartAudio(audioCallback);

  while (1) {
    // report audio callback CPU load once per second
    System::Delay(1000);
    DaisyPatchSM::PrintLine("CPU load avg: " FLT_FMT3 " max: " FLT_FMT3,
                            FLT_VAR3(cpuLoad.GetAvgCpuLoad()),
                            FLT_VAR3(cpuLoad.GetMaxCpuLoad()));
  }
}
