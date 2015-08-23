#include <Arduino.h>

typedef void (*Callback)(unsigned long period);

struct TapTempo {
    const unsigned long max_delay;
    const Callback callback;
    unsigned long last_tap = 0;
    unsigned long ndelays = 0;
    unsigned long delay0 = 0;
    unsigned long delay1 = 0;

    TapTempo(unsigned long max_delay, Callback callback) : max_delay(max_delay), callback(callback) {}

    void tap() {
        unsigned long delay = micros() - last_tap;
        last_tap = micros();
        if (delay > max_delay) {
            ndelays = 0;
        } else {
            if (ndelays == 0) {
                ndelays = 1;
                delay0 = delay;
            } else if (ndelays == 1) {
                ndelays = 2;
                delay1 = delay;
                delay = (delay0 + delay1) / 2;
            } else {
                delay0 = delay0 / 4 + delay1 * 3 / 4;
                delay1 = delay;
                delay = (delay0 + delay1) / 2;
            }

            callback(delay);
        }
    }
};
