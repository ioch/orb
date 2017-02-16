#include <EEPROM.h>

#include <Messenger.h>
#include "color.h"
#include "taptempo.h"

#define MONITOR_EXPRESSION(x) { Serial.print(#x ": "); Serial.println((x)); }

// Timeout for tap-tempo, in microseconds:
#define MAX_DELAY 2000000

#define ANALOG_MAX 1024

//~ #define LED_GND 8
#define LED_R 9
#define LED_G 10
#define LED_B 11

#define POT_RIGHT A1
#define POT_MIDDLE A2
#define POT_LEFT A3
//~ #define POT_GND A2
//~ #define POT_VCC A4

//~ #define BUTTON_GND 6
#define BUTTON A4
#define TERMISTOR_ENABLE 12
#define TERMISTOR A0

#define FAN_SENSE 5
#define FAN_CTRL 6


Messenger messenger;


enum OrbMode {
    MODE_DIRECT,
    MODE_PATTERN
} mode = MODE_DIRECT;

//~ #define PATTERN_SIZE 4
//~ rgb rgb_pattern[PATTERN_SIZE] = {
    //~ rgb(255, 255, 255), rgb(0, 255, 0), rgb(255, 255, 255), rgb(0, 255, 0),
//~ };
//~ int duty_pattern[PATTERN_SIZE] = {
    //~ 512, 1, 1, 1,
//~ };
//~ int pattern_index;


int duty = ANALOG_MAX / 2;

unsigned char saturation = 255;
unsigned char brightness = 255;

bool active = false;

unsigned long blink_period = 0;

unsigned long last_on = 0;
unsigned long next_on = 0;
unsigned long next_off = 0;

unsigned char fanpwm = 255;

unsigned int  temperature = 0;

void reschedule() {
    const unsigned long off_delay = (unsigned long)(blink_period * duty / ANALOG_MAX);
    next_off = last_on + off_delay;
    next_on = last_on + blink_period;
}

void set_color(rgb c) {
    analogWrite(LED_R, c.r);
    analogWrite(LED_G, c.g);
    analogWrite(LED_B, c.b);
}

void on() {
    switch (mode) {
        case MODE_DIRECT:
            set_color(
                hsv(random(255),
                    saturation,
                    brightness));
            break;

        case MODE_PATTERN:
            //~ set_color(rgb_pattern[pattern_index].scaled(brightness));
            //~ duty = duty_pattern[pattern_index];
            //~ pattern_index = (pattern_index + 1) % PATTERN_SIZE;
            break;
    }

    last_on = micros();
    reschedule();
}

void off() {
    set_color(rgb(0, 0, 0));
}


void message_ready() {
    while (messenger.available()) {
        messenger.readInt();
//        mode = MODE_PATTERN;
//        for (int i = 0; i < PATTERN_SIZE; ++i) {
//            duty_pattern[i] = messenger.readInt();
//        }
    }
}


void setup() {
    Serial.begin(115200); 
    messenger.attach(message_ready);

    // 32kHz PWM frequency
    TCCR1B = (TCCR1B & 0b11111000) | 1;
    TCCR2B = (TCCR2B & 0b11111000) | 1;


    pinMode(BUTTON, INPUT_PULLUP);
  	pinMode(TERMISTOR_ENABLE, OUTPUT);
  	digitalWrite(TERMISTOR_ENABLE, HIGH);
  	pinMode(TERMISTOR, INPUT);
  	pinMode(FAN_SENSE, INPUT_PULLUP);
  	pinMode(FAN_CTRL, OUTPUT);
  	analogWrite(FAN_CTRL, 255);
	

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
    duty = ANALOG_MAX - analogRead(POT_LEFT);
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

void get_fan_speed(){
}
unsigned long datapoints = 0;

void get_led_temp(){
  int readings = analogRead(TERMISTOR);
  temperature = (temperature - (temperature - readings) >>2);
  datapoints ++;
  if (datapoints >= 1000){
      Serial.println(temperature);
      datapoints = 0;
  }
}

void loop() {
    unsigned long t = micros();
    analogWrite(FAN_CTRL, fanpwm);
    get_fan_speed();
    get_led_temp();

    if (active && t > next_on) {
        on();
    }
    if (t > next_off) {
        off();
    }

    read_control();

    while (Serial.available()) {
        messenger.process(Serial.read());
    }

    delayMicroseconds(1);
}
