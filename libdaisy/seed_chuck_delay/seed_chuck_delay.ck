global float mix;
global float delay;
global float feedback;

800::ms => dur MAX_DELAY;

adc => Gain _dry => Gain _mix => dac;
adc => Gain _pre => Delay _delay => Gain _wet => _mix;
_wet => Gain _feedback => LPF lpf => _pre;

{
  MAX_DELAY => _delay.max;

  0.8 => mix;
  0.4 => delay;
  0.9 => feedback;

  1000.0 => lpf.freq;
}

while (true) {
  mix => _wet.gain;
  1.0f - mix => _dry.gain;
  delay * MAX_DELAY => _delay.delay;
  feedback => _feedback.gain;
  10::ms => now;
}
