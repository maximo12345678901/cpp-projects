#version 330 core

uniform vec2 u_coeff1;
uniform vec2 u_coeff2;
uniform int  u_degree;
uniform float u_sigma;
uniform float u_maxReal;
uniform float u_maxImag;
uniform vec2 u_resolution;

out vec4 fragColor;

vec4 hsv_to_rgb(vec3 color) {
    // Translates HSV color to RGB color
    // H: 0.0 - 360.0, S: 0.0 - 100.0, V: 0.0 - 100.0
    // R, G, B: 0.0 - 1.0

    float hue = color.x;
    float saturation = color.y;
    float value = color.z;

    float c = (value/100) * (saturation/100);
    float x = c * (1 - abs(mod(hue/60, 2) - 1));
    float m = (value/100) - c;

    float r = 0;
    float g = 0;
    float b = 0;
    
    if (hue >= 0 && hue < 60) {
        r = c;
        g = x;
        b = 0;
    } else if (hue >= 60 && hue < 120) {
        r = x;
        g = c;
        b = 0;
    } else if (hue >= 120 && hue < 180) {
        r = 0;
        g = c;
        b = x;
    } else if (hue >= 180 && hue < 240) {
        r = 0;
        g = x;
        b = c;
    } else if (hue >= 240 && hue < 300) {
        r = x;
        g = 0;
        b = c;
    } else if (hue >= 300 && hue < 360) {
        r = c;
        g = 0;
        b = x;
    }

    r += m;
    g += m;
    b += m;

    return vec4(r, g, b, 1.0);
}

vec2 c_add(vec2 a, vec2 b) {
    return a + b;
}

vec2 c_mul(vec2 a, vec2 b) {
    return vec2(
        a.x*b.x - a.y*b.y,
        a.x*b.y + a.y*b.x
    );
}
vec2 c_div(vec2 a, vec2 b) {
    float denom = b.x*b.x + b.y*b.y;
    return vec2(
        (a.x*b.x + a.y*b.y) / denom,
        (a.y*b.x - a.x*b.y) / denom
    );
}

vec2 c_pow(vec2 v, int p) {
    vec2 current = v;
    for (int i = 1; i < p; i++) {
        current = c_mul(current, v);
    }
    return current;
}

vec2 polynomial(vec2 z, int mask) {
    vec2 sum = vec2(0.0);

    vec2 zpow = vec2(1.0, 0.0); // z^0

    for (int i = 0; i < 32; i++) {
        if (i >= u_degree) break;

        int bit = (mask >> i) & 1;
        vec2 coeff = (bit == 1) ? u_coeff1 : u_coeff2;

        // Add coeff * z^i
        sum = c_mul(coeff, zpow) + sum;

        // z^(i+1)
        zpow = c_mul(zpow, z);
    }

    return sum;
}

vec2 polynomial_derivative(vec2 z, int mask) {
    vec2 sum = vec2(0.0);

    vec2 zpow = vec2(1.0, 0.0);   // z^0

    // We will use zpow = z^(i-1)
    for (int i = 1; i < 32; i++) {
        if (i >= u_degree) break;

        int bit = (mask >> i) & 1;
        vec2 coeff = (bit == 1) ? u_coeff1 : u_coeff2;

        // derivative term = i * coeff * z^(i-1)
        sum += c_mul(coeff * float(i), zpow);

        // next z^(i)
        zpow = c_mul(zpow, z);
    }

    return sum;
}



void main() {
    // Pixel to complex mapping
    vec2 uv = gl_FragCoord.xy;
    float real = (uv.x - u_resolution.x/2.0) / ((u_resolution.x/2.0)/u_maxReal);
    float imag = (uv.y - u_resolution.y/2.0) / ((u_resolution.y/2.0)/u_maxImag);
    vec2 z = vec2(real, imag);

    bool isPoint = false;

    int total = 1 << u_degree;

    for (int i = 0; i < total; i++) {
        vec2 value = polynomial(z, i);
        vec2 value_deriv = polynomial_derivative(z, i);

        float dist = length(c_div(value, value_deriv));
        float radius = u_sigma;    // fixed dot size
        if (dist < radius) {
            isPoint = true;

            vec4 color = hsv_to_rgb(vec3(360 * float(i) / float(total), 100.0, 100.0)); ////// THIS IS THE LINE WITH THE ERROR MISTER GPT
            fragColor = color;
        }
    }

    if (!isPoint) {
        fragColor = vec4(0.0);
    }
}
