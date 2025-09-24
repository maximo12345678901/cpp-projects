// render.frag
#version 330 core

out vec4 FragColor;
uniform sampler2D stateTex;
uniform vec2 gridSize;

vec3 stateColor(int s) {
    if (s == 1) return vec3(1.0);          // white
    if (s == 2) return vec3(1.0, 0.0, 0.0); // red
    if (s == 3) return vec3(0.0, 0.0, 1.0); // blue
    if (s == 4) return vec3(0.0, 1.0, 0.0); // green
    return vec3(0.0); // black for state 0
}

void main() {
    ivec2 texSize = textureSize(stateTex, 0);
    vec2 uv = gl_FragCoord.xy / vec2(texSize); // map window pixel to texel coordinates
    float r = texture(stateTex, uv).r;
    int s = int(r * 4.0 + 0.5);
    FragColor = vec4(stateColor(s), 1.0);
}
