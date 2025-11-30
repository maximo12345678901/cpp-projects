#version 330 core

uniform vec2 u_coeff1;       // (-50, 0)
uniform vec2 u_coeff2;       // (50, 0)
uniform int  u_degree;       // coefficients.size()
uniform float u_sigma;       // Gaussian size
uniform float u_maxReal;     // 3.0
uniform float u_maxImag;     // 3.0
uniform vec2 u_resolution;   // (screenDiameter, screenDiameter)

out vec4 fragColor;

//
// Complex helpers
//
vec2 c_add(vec2 a, vec2 b) {
    return a + b;
}

vec2 c_mul(vec2 a, vec2 b) {
    return vec2(
        a.x*b.x - a.y*b.y,
        a.x*b.y + a.y*b.x
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

    return sum * 10.0;
}


void main() {
    // Pixel to complex mapping
    vec2 uv = gl_FragCoord.xy;
    float real = (uv.x - u_resolution.x/2.0) / ((u_resolution.x/2.0)/u_maxReal);
    float imag = (uv.y - u_resolution.y/2.0) / ((u_resolution.y/2.0)/u_maxImag);
    vec2 z = vec2(real, imag);

    float sumIntensity = 0.0;

    int total = 1 << u_degree;

    for (int i = 0; i < total; i++) {
        vec2 r = polynomial(z, i);
        float dist = length(r);
        float intensity = exp(-(dist*dist) / (2.0*u_sigma*u_sigma));
        sumIntensity += intensity;
    }

    // Clamp to prevent blowout
    float v = clamp(sumIntensity, 0.0, 1.0);

    fragColor = vec4(v, v, v, 1.0);
}
