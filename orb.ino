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

#define FAN_SENSE 2
#define FAN_CTRL 6


Messenger messenger;


enum OrbMode {
    MODE_PARTY,
    MODE_LIGHT
} mode = MODE_PARTY;

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
    set_color(hsv(random(255),saturation,brightness));
    last_on = micros();
    reschedule();
}

void off() {
    set_color(rgb(0, 0, 0));
}


void message_ready() {
    while (messenger.available()) {
        fanpwm = 255-messenger.readInt();
        
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
    if (digitalRead(BUTTON) == LOW){
      mode = MODE_LIGHT;
    }

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

unsigned int pot_left = 0;
unsigned int pot_middle = 0;
unsigned int pot_right = 0;

void read_control() {
    pot_left =  pot_left -(pot_left -analogRead(POT_LEFT)) >>2 ;
    pot_middle =  pot_left -(pot_left -analogRead(POT_MIDDLE)) >>2;
    pot_right =  pot_left -(pot_left -analogRead(POT_RIGHT)) >> 2;
//    Serial.print("PL:");        //debug
//    Serial.print(pot_left);
//    Serial.print("PM:");
//    Serial.print(pot_middle);
//    Serial.print("PR:");
//    Serial.println(pot_right);              
}

unsigned long datapoints = 0;

void led_temp(){
  int readings = analogRead(TERMISTOR);
  temperature = (temperature - (temperature - readings) >>2);
  datapoints ++;
  if (datapoints >= 1000){
      Serial.print("T:");
      Serial.println(temperature);
      datapoints = 0;
  }
  if (temperature > 182){
    fanpwm = 0;
  }else if (temperature >170){
    fanpwm = 100;
  }else fanpwm = 255;
}

void manageLED(){
    unsigned long t = micros();
    static int last_duty = duty;
    duty = ANALOG_MAX - pot_left;
    if (abs(duty - last_duty) > 2) {
        reschedule();
    }
    last_duty = duty;

    saturation = max(255 - pot_middle / 4, 10);
    brightness = max(255 - pot_right / 4, 0);

    static bool last_button = 0;
    bool button = digitalRead(BUTTON);
    if (button < last_button) {
        tap_tempo.tap();
    }
    last_button = button;

    if (active && t > next_on) {
        on();
    }
    if (t > next_off) {
        off();
    }
  
}


void loop() {
    analogWrite(FAN_CTRL, fanpwm);
    led_temp();
    read_control();
    switch (mode) {
        case MODE_PARTY:
            manageLED();
            break;

        case MODE_LIGHT:
            set_color(rgb(pot_left/4,pot_middle/4,pot_right/4));
            break;
    }    



    while (Serial.available()) {
        messenger.process(Serial.read());
    }

    delayMicroseconds(1);
}
