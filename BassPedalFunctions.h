#include "daisy_seed.h"
using namespace daisy;

Color getLEDColor(float _inputLevel) {
    // Takes in input level in dB, gives color for LED
    Color c;
    float rVal, gVal, bVal = 0.0;    
    float b = 0.5;

    rVal = gVal = bVal = 0.0;
    if (_inputLevel > -40 && _inputLevel < -25) {
        //ideal
        gVal = b;
    } else if (_inputLevel > -25) {
        rVal = b;
    } else {
        bVal = b;
    }

    c.Init(rVal, gVal, bVal);
    return c;
}

inline double dB2Raw(double x) {
    return pow(10.0, x / 20.0);
}

inline double raw2dB(double x) {
    return 20 * log10(x);
}
