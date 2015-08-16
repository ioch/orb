#define LED_R 9
#define LED_G 10
#define LED_B 11

#define TAP_TEMPO_BUTTON 0
#define DUTY_POT 0

void on() {
    unsigned char r, g, b;
    unsigned char h = random(255);
    unsigned char s = 255;
    unsigned char v = 255;
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

unsigned long last_on = 0;
unsigned long off_delay = 42;
unsigned long on_delay = 42;

void reschedule() {
    next_off = last_on + off_delay;
    next_on = last_on + on_delay;
}

void setup() {                
    // initialize the digital pin as an output.
    pinMode(8, OUTPUT);
    digitalWrite(8, LOW); 
    pinMode(LED_R, OUTPUT);
    digitalWrite(LED_R, LOW);
    pinMode(LED_G, OUTPUT);
    digitalWrite(LED_G, LOW);
    pinMode(LED_B, OUTPUT);
    digitalWrite(LED_B, LOW);

    reschedule();
}

void read_control() {
}

void loop() {
    unsigned long t = micros();

    if (millis() > next_on) {
        on();
        last_on = millis();
    } else if (millis() > next_off) {
        off();
    } else {
        read_control();
    }

    delay(1);
}
