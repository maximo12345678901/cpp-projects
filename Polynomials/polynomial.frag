#version 330 core

uniform vec2 u_coeff1;
uniform vec2 u_coeff2;
uniform int  u_degree;
uniform float u_sigma;
uniform float u_maxReal;
uniform float u_maxImag;
uniform vec2 u_resolution;

out vec4 fragColor;

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

    float sumIntensity = 0.0;

    int total = 1 << u_degree;

    for (int i = 0; i < total; i++) {
        vec2 value = polynomial(z, i);
        vec2 value_deriv = polynomial_derivative(z, i);

        float dist = length(c_div(value, value_deriv));
        float radius = u_sigma;    // fixed dot size
        float intensity = dist < radius ? 1.0 : 0.0;

        sumIntensity += intensity;
    }

    // Clamp to prevent blowout
    float v = clamp(sumIntensity, 0.0, 1.0);

    fragColor = vec4(v, v, v, 1.0);
}
