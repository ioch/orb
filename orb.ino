#include <Messenger.h>
#include "color.h"
#include "taptempo.h"

#define MONITOR_EXPRESSION(x) { Serial.print(#x ": "); Serial.println((x)); }

// Timeout for tap-tempo, in microseconds:
#define MAX_DELAY 2000000

#define ANALOG_MAX 1024

#define LED_GND 8
#define LED_R 9
#define LED_G 10
#define LED_B 11

#define POT_RIGHT A0
#define POT_MIDDLE A1
#define POT_LEFT A3
#define POT_GND A2
#define POT_VCC A4

#define BUTTON_GND 6
#define BUTTON 7


Messenger messenger;


enum OrbMode {
    MODE_DIRECT,
    MODE_PATTERN
} mode = MODE_DIRECT;

int duty = ANALOG_MAX / 2;

unsigned char saturation = 255;
unsigned char brightness = 255;

bool active = false;

unsigned long blink_period = 0;

unsigned long last_on = 0;
unsigned long next_on = 0;
unsigned long next_off = 0;


void reschedule() {
    const unsigned long off_delay = (unsigned long)(blink_period * (ANALOG_MAX - duty) / ANALOG_MAX);
    next_off = last_on + off_delay;
    next_on = last_on + blink_period;
}

void set_color(rgb c) {
    analogWrite(LED_R, c.r);
    analogWrite(LED_G, c.g);
    analogWrite(LED_B, c.b);
}

void on() {
    set_color(
        hsv(random(255),
            saturation,
            brightness));

    last_on = micros();
    reschedule();
}

void off() {
    set_color(rgb(0, 0, 0));
}


void setup() {
    // 32kHz PWM frequency
    TCCR1B = (TCCR1B & 0b11111000) | 1;
    TCCR2B = (TCCR2B & 0b11111000) | 1;

    pinMode(POT_GND, OUTPUT);
    digitalWrite(POT_GND, LOW); 
    pinMode(POT_VCC, OUTPUT);
    digitalWrite(POT_VCC, HIGH); 

    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(BUTTON_GND, OUTPUT);
    digitalWrite(BUTTON_GND, LOW); 

    pinMode(LED_GND, OUTPUT);
    digitalWrite(LED_GND, LOW); 
    pinMode(LED_R, OUTPUT);
    digitalWrite(LED_R, LOW);
    pinMode(LED_G, OUTPUT);
    digitalWrite(LED_G, LOW);
    pinMode(LED_B, OUTPUT);
    digitalWrite(LED_B, LOW);

    reschedule();
}

void set_period(unsigned long period) {
    if (!active) {
        active = true;
        last_on = micros();
    }

    blink_period = period;
    reschedule();
}

TapTempo tap_tempo(MAX_DELAY, set_period);

void read_control() {
    static int last_duty = duty;
    duty = analogRead(POT_LEFT);
    if (abs(duty - last_duty) > 2) {
        reschedule();
    }
    last_duty = duty;

    saturation = max(255 - analogRead(POT_MIDDLE) / 4, 10);
    brightness = max(255 - analogRead(POT_RIGHT) / 4, 0);

    static bool last_button = 0;
    bool button = digitalRead(BUTTON);
    if (button < last_button) {
        tap_tempo.tap();
    }
    last_button = button;
}

void loop() {
    unsigned long t = micros();

    if (active && t > next_on) {
        on();
    }
    if (t > next_off) {
        off();
    }

    read_control();

    delayMicroseconds(1);
}
