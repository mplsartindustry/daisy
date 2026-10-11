#include "fmu.h"

void Fmu::Init(float samplerate)
{
  car_.Init(samplerate);
  har_.Init(samplerate);
  mod_.Init(samplerate);

  lfreq_ = 440.f;
  lharmonic_ = 7.f;
  lratio_ = 2.f;
  SetFreq(lfreq_);
  SetHarmonic(lharmonic_);
  SetRatio(lratio_);
  SetDepth(1.f);

  car_.SetAmp(1.f);
  har_.SetAmp(1.f);
  mod_.SetAmp(1.f);

  car_.SetWaveform(Oscillator::WAVE_SIN);
  har_.SetWaveform(Oscillator::WAVE_SIN);
  mod_.SetWaveform(Oscillator::WAVE_SIN);
}

tuple Fmu::Process()
{
  if (lratio_ != ratio_ || lharmonic_ != harmonic_ || lfreq_ != freq_)
  {
    lratio_ = ratio_;
    lharmonic_ = harmonic_;
    lfreq_ = freq_;
    car_.SetFreq(lfreq_);
    har_.SetFreq(lfreq_ * lharmonic_);
    mod_.SetFreq(lfreq_ * lratio_);
  }

  float modval = mod_.Process();
  car_.PhaseAdd(modval * depth_);
  har_.PhaseAdd(modval * depth_);

  tuple t;
  t.a = car_.Process();
  t.b = har_.Process();
  return t;
}

void Fmu::SetFreq(float freq)
{
  freq_ = fabsf(freq);
}

void Fmu::SetHarmonic(float harmonic)
{
  harmonic_ = fabsf(harmonic);
}

void Fmu::SetRatio(float ratio)
{
  ratio_ = fabsf(ratio);
}

void Fmu::SetDepth(float depth)
{
  depth_ = fabsf(depth);
}

void Fmu::Reset()
{
  car_.Reset();
  mod_.Reset();
  har_.Reset();
}
