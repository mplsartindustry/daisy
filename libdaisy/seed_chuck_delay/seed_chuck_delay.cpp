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
#include "chuck.h"
#include "chuck_globals.h"
#include "sdram_heap.h"

using namespace daisy;
using namespace mplsartindustry;

MplsArtIndustrySeed hw;

ChucK * chuck = NULL;
SAMPLE * inputBuffer = NULL;
SAMPLE * outputBuffer = NULL;

// ChucK global floats, read directly from the VM
t_CKFLOAT * mixPtr = NULL;
t_CKFLOAT * delayPtr = NULL;
t_CKFLOAT * feedbackPtr = NULL;

// chuck code, from seed_chuck_delay.ck wrapped in a raw string literal at build time, see Makefile
const char * CHUCK_CODE =
#include "seed_chuck_delay_ck.h"
;

void audioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {

  hw.ProcessAllControls();

  bool hard = hw.sw1.On();

  // led1 shows hard clip
  hw.led1.Set(hard);

  float mix = hw.pot1.Value();
  float delay = hw.pot2.Value();
  float feedback = hw.pot3.Value();

  // led2 shows mix level
  hw.led2.Set(mix);

  // share parameters with ChucK vm
  *mixPtr = mix;
  *delayPtr = delay;
  *feedbackPtr = feedback;

  // interleave input buffers
  for (size_t i = 0; i < size; i++) {
     inputBuffer[2 * i] = in[0][i];
    inputBuffer[2 * i + 1] = in[1][i];
  }

  // run ChucK for the next `size` frames
  chuck->run(inputBuffer, outputBuffer, size);

  // deinterleave and clip output buffers
  for (size_t i = 0; i < size; i++) {
    out[0][i] = hard ? hw.HardClip(outputBuffer[2 * i]) : hw.SoftClip(outputBuffer[2 * i]);
    out[1][i] = hard ? hw.HardClip(outputBuffer[2 * i + 1]) : hw.SoftClip(outputBuffer[2 * i + 1]);
  }
}

int main(void) {

  // initialize hardware, including SDRAM
  hw.Init();
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);

  // ChucK allocates several MB, more than internal SRAM
  EnableSdramHeap();

  // pot1, mix
  hw.pot1.SetRange(0.0f, 1.0f, Parameter::LINEAR);

  // pot2, delay time
  hw.pot2.SetRange(0.0f, 1.0f, Parameter::LINEAR);

  // pot3, feedback level
  hw.pot3.SetRange(0.0f, 1.0f, Parameter::LINEAR);

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
  chuck->compileCode(CHUCK_CODE, "", 1);

  // create the globals now, the VM only creates them on its first run(), after the first audio callback reads them
  chuck->globals()->init_global_float("mix");
  chuck->globals()->init_global_float("delay");
  chuck->globals()->init_global_float("feedback");

  // write globals directly in the audio callback, the same thread as run()
  mixPtr = chuck->globals()->get_ptr_to_global_float("mix");
  delayPtr = chuck->globals()->get_ptr_to_global_float("delay");
  feedbackPtr = chuck->globals()->get_ptr_to_global_float("feedback");

  hw.StartAudio(audioCallback);
}
