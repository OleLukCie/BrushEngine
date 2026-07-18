#include "input_sample.h"
#include <windows.h>

InputSample makeInputSample(int x, int y, float pressure) {
    InputSample sample;
    sample.x = static_cast<float>(x);
    sample.y = static_cast<float>(y);
    sample.pressure = pressure;
    sample.timestamp = static_cast<float>(GetTickCount()) / 1000.0f;
    return sample;
}