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
#include "fm.h"

DaisyHardware patch;
Switch b8;

Fm fm;

void audioCallback(float** in, float** out, size_t size) {

  patch.ProcessAllControls();

  b8.Debounce();
  bool hard = b8.Pressed();

  // 0.0f to 1.0f
  float tunePot = truncf(patch.controls[0].Value()); // truncate to 3 digits
  float fineTunePot = truncf(patch.controls[1].Value()); // truncate to 3 digits
  float depthPot = patch.controls[2].Value();
  float ratioPot = patch.controls[3].Value();

  // -1.0f to 1.0f
  float freqCv = patch.controls[4].Value();
  float depthCv = patch.controls[5].Value();
  float ratioCv = patch.controls[6].Value();

  // 1 v per octave
  float freqVolts = freqCv * 5.0f; // -5.0 v to +5.0 v
  float tuneVolts = (tunePot - 0.5f) * 2.5f; // -1.25 v to +1.25 v 
  float fineTuneVolts = (fineTunePot - 0.5f) * 0.125f; // -0.0625 v to +0.0625 v
  float f = vtof(freqVolts + tuneVolts + fineTuneVolts);

  float d = depthCv + depthPot;
  float r = ratioCv + ratioPot;

  // cv_out_2, C1 led on front panel
  analogWrite(PIN_PATCH_SM_CV_OUT_2, led(d));

  fm.SetFreq(f);
  fm.SetDepth(depthCv + depthPot);
  fm.SetRatio(ratioCv + ratioPot);

  // process audio buffers
  for (size_t i = 0; i < size; i++) {
    float v = fm.Process();
    out[0][i] = out[1][i] = hard ? hardClip(v) : softClip(v);
  }
}

void setup() {

  // initialize hardware
  patch = DAISY.init(DAISY_PATCH_SM, AUDIO_SR_96K);

  // b8, toggle switch, hard clip/soft clip
  b8.Init(1000, true, PIN_PATCH_SM_B8, INPUT_PULLUP);

  // initialize oscillators
  fm.Init(DAISY.AudioSampleRate());

  DAISY.StartAudio(audioCallback);
}

void loop() {
  // empty
}

float vtof(float v) {  
  return powf(2, v) * 261.625565f;
}

float truncf(float f) {
  return roundf(f * 1000.0f) / 1000.0f;
}

int led(float f) {
  if (f < 0.0f) {
    return 0;
  }
  return (int) 1023.0f * f;
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
