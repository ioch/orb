#define MAX_DELAY 2000

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


int duty = 512;

unsigned char saturation = 255;
unsigned char brightness = 255;

unsigned long off_delay = 42;
unsigned long on_delay = 84;

unsigned long last_on = 0;
unsigned long next_on = 0;
unsigned long next_off = 0;




void on() {
    unsigned char r, g, b;
    unsigned char h = random(255);
    unsigned char s = saturation;
    unsigned char v = brightness;
    unsigned char region, fpart, p, q, t;

    if(s == 0) {
        /* color is grayscale */
        r = g = b = v;
        return;
    }

    /* make hue 0-5 */
    region = h / 43;
    /* find remainder part, make it from 0-255 */
    fpart = (h - (region * 43)) * 6;

    /* calculate temp vars, doing integer multiplication */
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * fpart) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;

    /* assign temp vars based on color cone region */
    switch(region) {
        case 0:
            r = v; 
            g = t; 
            b = p; 
            break;
        case 1:
            r = q; 
            g = v; 
            b = p; 
            break;
        case 2:
            r = p; 
            g = v; 
            b = t; 
            break;
        case 3:
            r = p; 
            g = q; 
            b = v; 
            break;
        case 4:
            r = t; 
            g = p; 
            b = v; 
            break;
        default:
            r = v; 
            g = p; 
            b = q; 
            break;
    }

    analogWrite(LED_R, r);
    analogWrite(LED_G, g);
    analogWrite(LED_B, b);
}

void off() {
    analogWrite(LED_R, 0);
    analogWrite(LED_G, 0);
    analogWrite(LED_B, 0);
}

void reschedule() {
    next_off = last_on + off_delay;
    next_on = last_on + on_delay;
}

void setup() {
    // analog
    //pinMode(POT_LEFT, INPUT);
    //pinMode(POT_MIDDLE, INPUT);
    //pinMode(POT_RIGHT, INPUT);
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

void reset_off() {
    float dutyf = 1.0 - duty / 1024.0;
    off_delay = (unsigned long)(on_delay * dutyf);
}

int ndelays = 0;
int delay0 = 0;
int delay1 = 0;

void read_control() {
    static int last_duty = duty;
    duty = analogRead(POT_LEFT);
    if (abs(duty - last_duty) > 2) {
        reset_off();
        reschedule();
    }
    last_duty = duty;

    saturation = max(255 - analogRead(POT_MIDDLE) / 4, 10);
    brightness = max(255 - analogRead(POT_RIGHT) / 4, 0);

    static unsigned long last_tap = 0;
    static bool last_button = 0;
    bool button = digitalRead(BUTTON);
    if (button < last_button) {
        unsigned long delay = millis() - last_tap;
        last_tap = millis();
        if (delay < MAX_DELAY) {
            if (ndelays == 0) {
                ndelays = 1;
                delay0 = delay;
            } else if (ndelays == 1) {
                ndelays = 1;
                delay1 = delay;
                delay = (delay0 + delay1) / 2;
            } else {
                delay0 = delay1;
                delay1 = delay;
                delay = (delay0 + delay1) / 2;
            }

            on_delay = delay / 2;
            reset_off();
            reschedule();
        } else {
            ndelays = 0;
        }
    }
    last_button = button;
}

void loop() {
    unsigned long t = millis();

    if (t > next_on) {
        on();
        last_on = t;
        reschedule();
    }
    if (t > next_off) {
        off();
    }

    read_control();

    delay(1);
}
