global float mix;
global float delay;
global float feedback;

220::ms => dur MAX_DELAY;

adc => Gain _dry => Gain _mix => dac;
adc => Gain _pre => Delay _delay => Gain _wet => _mix;
_wet => Gain _feedback => _pre;

{
  MAX_DELAY => _delay.max;

  0.8 => mix;
  0.4 => delay;
  0.6 => feedback;
}

while (true) {
  mix => _wet.gain;
  1.0f - mix => _dry.gain;
  feedback => _feedback.gain;
  delay * MAX_DELAY => _delay.delay;
  10::ms => now;
}
