#include "fm.h"

void Fm::Init(float samplerate)
{
  car_.Init(samplerate);
  mod_.Init(samplerate);

  lfreq_ = 440.f;
  lratio_ = 2.f;
  SetFreq(lfreq_);
  SetRatio(lratio_);
  SetDepth(1.f);

  car_.SetAmp(1.f);
  mod_.SetAmp(1.f);

  car_.SetWaveform(Oscillator::WAVE_SIN);
  mod_.SetWaveform(Oscillator::WAVE_SIN);
}

float Fm::Process()
{
  if (lratio_ != ratio_ || lfreq_ != freq_)
  {
    lratio_ = ratio_;
    lfreq_ = freq_;
    car_.SetFreq(lfreq_);
    mod_.SetFreq(lfreq_ * lratio_);
  }

  float modval = mod_.Process();
  if (depth_ > 0.05f) {
    car_.PhaseAdd(modval * depth_);
  }
  return car_.Process();
}

void Fm::SetFreq(float freq)
{
  freq_ = fabsf(freq);
}

void Fm::SetRatio(float ratio)
{
  ratio_ = fabsf(ratio);
}

void Fm::SetDepth(float depth)
{
  depth_ = fabsf(depth);
}

void Fm::Reset()
{
  car_.Reset();
  mod_.Reset();
}
