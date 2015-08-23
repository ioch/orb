struct rgb {
    const unsigned char r;
    const unsigned char g;
    const unsigned char b;

    rgb(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {}

    rgb scaled(unsigned char scale) {
        return rgb(
            r * scale / 255,
            g * scale / 255,
            b * scale / 255);
    }
};


rgb hsv(unsigned char h, unsigned char s, unsigned char v) {
    unsigned char r, g, b;
    unsigned char region, fpart, p, q, t;

    if(s == 0) {
        /* color is grayscale */
        r = g = b = v;
        return rgb(r, g, b);
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

    return rgb(r, g, b);
}

