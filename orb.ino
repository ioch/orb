int ledR = 9;
int ledG = 10;
int ledB = 11;

void setup() {                
    // initialize the digital pin as an output.
    pinMode(8, OUTPUT);
    digitalWrite(8, LOW); 
    pinMode(ledR, OUTPUT);
    digitalWrite(ledR, LOW);
    pinMode(ledG, OUTPUT);
    digitalWrite(ledG, LOW);
    pinMode(ledB, OUTPUT);
    digitalWrite(ledB, LOW);
}

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

    analogWrite(ledR, r);
    analogWrite(ledG, g);
    analogWrite(ledB, b);
}

void off() {
    analogWrite(ledR, 0);
    analogWrite(ledG, 0);
    analogWrite(ledB, 0);
}

unsigned long last_on = 0;
unsigned long off_delay = 42;
unsigned long on_delay = 42;

void loop() {
    delay(1);
}
