#include <stdint.h>
#include "DaisyDuino.h"

class Fm
{
  public:
    Fm() {}
    ~Fm() {}

    void Init(float samplerate);

    float Process();

    void SetFreq(float freq);

    void SetHarmonic(float harmonic);

    void SetRatio(float ratio);

    void SetDepth(float depth);

    void Reset();

  private:
    Oscillator mod_, car_;
    float      depth_;
    float      freq_, lfreq_, harmonic_, lharmonic_, ratio_, lratio_;
};
