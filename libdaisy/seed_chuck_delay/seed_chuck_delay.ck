global float dry;
global float wet;
global float delay;
global float cutoff;
global float feedback;

0.0f => float lastDelay;

800::ms => dur MAX_DELAY;

adc => Gain _dry => Gain _mix => dac;
adc => Gain _pre => Delay _delay => Gain _wet => _mix;
_wet => Gain _feedback => LPF _lpf => _pre;

{
  MAX_DELAY => _delay.max;

  0.8 => wet;
  0.2 => dry;
  0.4 => delay;
  1600.0 => cutoff;
  0.9 => feedback;
}

256::samp => now;

while (true) {
  wet => _wet.gain;
  dry => _dry.gain;
  cutoff => _lpf.freq;
  feedback => _feedback.gain;

  if (Math.fabs(delay - lastDelay) > 0.001f) {
    delay * MAX_DELAY => _delay.delay;
    delay => lastDelay;
  }

  512::samp => now;
}
