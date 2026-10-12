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
#include "DaisyDuino.h"
#include "chuck.h"
#include "chuck_globals.h"

DaisyHardware patch;
Switch b7;
Switch b8;

// ChucK instance
ChucK * chuck = NULL;

// interleaved audio buffers, as expected by ChucK::run()
SAMPLE * inputBuffer = NULL;
SAMPLE * outputBuffer = NULL;
size_t bufferSize = 0;

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

void audioCallback(float** in, float** out, size_t size) {

  patch.ProcessAllControls();

  b7.Debounce();
  b8.Debounce();

  bool rising = b7.RisingEdge();
  bool pressed = b7.Pressed();
  bool hard = b8.Pressed();

  if (rising) {
    chuck->globals()->broadcastGlobalEvent("trigger");
  }

  // cv_out_2, C1 led on front panel
  digitalWrite(PIN_PATCH_SM_CV_OUT_2, hard);

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
}

void chuckPrint(const char * message) {
  Serial.print(message);
}

void setup() {

  // initialize hardware
  patch = DAISY.init(DAISY_PATCH_SM, AUDIO_SR_96K);

  // b7, momentary button B7, trigger
  b7.Init(1000, true, PIN_PATCH_SM_B7, INPUT_PULLUP);

  // b8, toggle switch, hard clip/soft clip
  b8.Init(1000, true, PIN_PATCH_SM_B8, INPUT_PULLUP);

  Serial.begin(9600);

  // route ChucK console output to serial
  ChucK::setStdoutCallback(chuckPrint);
  ChucK::setStderrCallback(chuckPrint);

  // initialize ChucK
  chuck = new ChucK();

  chuck->setParam(CHUCK_PARAM_SAMPLE_RATE, (t_CKINT) DAISY.get_samplerate());
  chuck->setParam(CHUCK_PARAM_INPUT_CHANNELS, 2);
  chuck->setParam(CHUCK_PARAM_OUTPUT_CHANNELS, 2);
  chuck->setParam(CHUCK_PARAM_VM_HALT, FALSE);
  chuck->setParam(CHUCK_PARAM_IS_REALTIME_AUDIO_HINT, TRUE);
  chuck->setParam(CHUCK_PARAM_CHUGIN_ENABLE, FALSE);

  // allocate audio buffers, after parameters are set
  bufferSize = DAISY.AudioBlockSize();
  inputBuffer = new SAMPLE[bufferSize * 2]();
  outputBuffer = new SAMPLE[bufferSize * 2]();

  chuck->init();
  chuck->start();

  chuck->compileCode(CHUCK_CODE, "", 1);

  DAISY.StartAudio(audioCallback);
}

void loop() {
  // empty
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
