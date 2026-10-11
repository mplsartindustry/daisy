#include <stdint.h>
#include "DaisyDuino.h"

struct tuple
{
  float a;
  float b;
};

class Fmu
{
  public:
    Fmu() {}
    ~Fmu() {}

    void Init(float samplerate);

    tuple Process();

    void SetFreq(float freq);

    void SetHarmonic(float harmonic);

    void SetRatio(float ratio);

    void SetDepth(float depth);

    void Reset();

  private:
    Oscillator mod_, car_, har_;
    float      depth_;
    float      freq_, lfreq_, harmonic_, lharmonic_, ratio_, lratio_;
};
