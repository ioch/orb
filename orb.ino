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
    MODE_LIGHT,
    MODE_COMM
} mode = MODE_PARTY;



int duty = ANALOG_MAX / 2;

unsigned char saturation = 255;
unsigned char brightness = 255;

bool active = false;

unsigned long blink_period = 0;

unsigned long last_on = 0;
unsigned long next_on = 0;
unsigned long next_off = 0;

unsigned char fanpwm = 255;

signed int pot_left = 0;
signed int pot_middle = 0;
signed int pot_right = 0;

signed int  temperature = 0;

unsigned long t = 0;

//void(* resetFunc) (void) = 0; //declare reset function @ address 0
void resetFunc(){
  asm("jmp 0x3800");
}

void reschedule() {
    const unsigned long off_delay = (unsigned long)(blink_period * duty / ANALOG_MAX);
    next_off = last_on + off_delay;
    next_on = last_on + blink_period;
}

unsigned int red = 0;
unsigned int green = 0;
unsigned int blue = 0;

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
      unsigned char task = messenger.readChar();
      switch (task){
        case 0x46: // F Fan
          fanpwm= 255-messenger.readInt();
          Serial.print("FAN:");
          Serial.println(255-fanpwm);
          break;
        case 0x52:  //R Reset
          Serial.println("RESETING");
          delay(1);
          resetFunc();
          break; 
        case 0x4d:  //M Mode (valid 0, 1, 2)
          Serial.println(mode);
          mode = messenger.readInt();
          Serial.print("MODE:");
          Serial.println(mode);
          break;
        case 0x43: //C COLOR (valid rr gg bb)
          red = messenger.readInt();
          green = messenger.readInt();
          blue = messenger.readInt();
          Serial.print("COLOR:");
          Serial.print(red);
          Serial.print(" ");
          Serial.print(green);
          Serial.print(" ");
          Serial.println(blue);
          break;
        case 0x53: //S Save  - save rgb to eeprom
          Serial.print("writing EEPROM! ");
          EEPROM.write(0, red);
          EEPROM.write(1, green);
          EEPROM.write(2, blue);
          Serial.println("Done");
          break;
        case 0x47: // G Get from eeprom
          red = EEPROM.read(0);
          green = EEPROM.read(1);
          blue = EEPROM.read(2);
          Serial.print("COLOR:");
          Serial.print(red);
          Serial.print(" ");
          Serial.print(green);
          Serial.print(" ");
          Serial.println(blue);
          break;
        case 0x48: //H Help
          Serial.println("H Help");
          Serial.println("R Reset");
          Serial.println("C <r> <g> <b> Color red green blue");
          Serial.println("M <0-2> Mode party, analog remote");
          Serial.println("S Save to eeprom");
          Serial.println("G Get from eeprom"); 
        break;        
        case 0x50: //P poteciometer print
          Serial.print("PL:");        //debug
          Serial.print(pot_left);
          Serial.print("PM:");
          Serial.print(pot_middle);
          Serial.print("PR:");
          Serial.println(pot_right);           
          break;
        case 0x54: //T Temperature
          Serial.print("T:");
          Serial.println(temperature);
          break;
      }
        
//        mode = MODE_PATTERN;
//        for (int i = 0; i < PATTERN_SIZE; ++i) {
//            duty_pattern[i] = messenger.readInt();
//        }
    }
}


void setup() {
    Serial.begin(57600); 
    messenger.attach(message_ready);

    // 32kHz PWM frequency
    TCCR1B = (TCCR1B & 0b11111000) | 1;
    TCCR2B = (TCCR2B & 0b11111000) | 1;


    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(LED_R, OUTPUT);
    digitalWrite(LED_R, LOW);
    pinMode(LED_G, OUTPUT);
    digitalWrite(LED_G, LOW);
    pinMode(LED_B, OUTPUT);
    digitalWrite(LED_B, LOW);
    pinMode(FAN_CTRL, OUTPUT);
    analogWrite(FAN_CTRL, 255);
  	pinMode(TERMISTOR_ENABLE, OUTPUT);
  	digitalWrite(TERMISTOR_ENABLE, HIGH);
  	pinMode(TERMISTOR, INPUT);
  	pinMode(FAN_SENSE, INPUT_PULLUP);
    red = EEPROM.read(0);
    green = EEPROM.read(1);
    blue = EEPROM.read(2);
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

void read_control() {
    pot_left =  pot_left +((analogRead(POT_LEFT) -pot_left) >>2) ;
    pot_middle =  pot_middle +((analogRead(POT_MIDDLE) -pot_middle) >>2);
    pot_right =  pot_right +((analogRead(POT_RIGHT) -pot_right) >> 2);          
}

unsigned long datapoints = 0;

void led_temp(){
  int readings = analogRead(TERMISTOR);
  temperature = (temperature + ((readings - temperature) >>2));
  datapoints ++;
  if (datapoints >= 1000){
//      Serial.print("T:");
//      Serial.println(temperature);
  if (temperature > 900){ //980 is ~100C 900 is ~65C
    mode = 2;
    red = 0;
    green = 0;
    blue = 0;
    Serial.print("Termal shutdown, T:");
    Serial.println(temperature);
  }else  if (temperature > 800){ //800 is ~ 50C
    fanpwm = 0;
  }else if (temperature >700){ //700 is ~40C 
    fanpwm = 100;
  }else fanpwm = 255;
  datapoints = 0;
  }
}

void manageLED(){

    static int last_duty = duty;
    duty = ANALOG_MAX - pot_left;
    if (abs(duty - last_duty) > 2) {
        reschedule();
    }
    last_duty = duty;

    saturation = max(255 - pot_middle / 4, 10);
    brightness = max(255 - pot_right / 4, 0);




    if (active && t > next_on) {
        on();
    }
    if (t > next_off) {
        off();
    }
  
}


void loop() {
    t = micros();
    analogWrite(FAN_CTRL, fanpwm);
    led_temp();
    read_control();
    static bool last_button = 0;
    bool button = digitalRead(BUTTON);
    switch (mode) {
        case MODE_PARTY:
            manageLED();
            if (button < last_button) {
              tap_tempo.tap();
            }
            break;

        case MODE_LIGHT:
            set_color(rgb(pot_left/4,pot_middle/4,pot_right/4));
            if (button < last_button) {
              mode = MODE_COMM;
            }
            break;
        case MODE_COMM:
            set_color(rgb(red,green,blue));
            if (button < last_button) {
              mode = MODE_LIGHT;
            }
            break;
    }    
    last_button = button;   


    while (Serial.available()) {
        messenger.process(Serial.read());
    }

    delayMicroseconds(1);
}
