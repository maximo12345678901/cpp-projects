// update.frag
#version 330 core

out vec4 FragColor;

uniform sampler2D stateTex;
uniform vec2 gridSize; // passed from CPU (floats)

struct Rule {
    int currentState;
    int nextState;
    int requiredNeighbors;
    int neighborType;
};
uniform Rule rules[32];
uniform int ruleCount;

// read cell state [0..4] from texel (1 texel == 1 cell)
// we encode states as red channel = state / 4.0
int getState(ivec2 pos, ivec2 gsize) {
    if (pos.x < 0 || pos.y < 0 || pos.x >= gsize.x || pos.y >= gsize.y)
        return 0;
    vec2 uv = (vec2(pos) + 0.5) / vec2(gsize);
    float r = texture(stateTex, uv).r;
    return int(r * 4.0 + 0.5);
}

void main() {
    ivec2 gsize = ivec2(gridSize + 0.5);
    ivec2 cell = ivec2(gl_FragCoord.xy); // gl_FragCoord.xy gives pixel coords in the render target
    int current = getState(cell, gsize);

    // neighbor counts for states 0..4
    int neighborCounts[5];
    for (int i = 0; i < 5; ++i) neighborCounts[i] = 0;

    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            ivec2 npos = cell + ivec2(dx, dy);
            if (npos.x < 0 || npos.y < 0 || npos.x >= gsize.x || npos.y >= gsize.y) continue;
            int s = getState(npos, gsize);
            if (s >= 0 && s <= 4) neighborCounts[s]++;
        }
    }

    int nextState = current;
    for (int i = 0; i < ruleCount; ++i) {
        if (current == rules[i].currentState) {
            if (neighborCounts[rules[i].neighborType] == rules[i].requiredNeighbors) {
                nextState = rules[i].nextState;
                break;
            }
        }
    }

    // encode nextState as red channel (0..4) -> 0..1
    FragColor = vec4(float(nextState) / 4.0, 0.0, 0.0, 1.0);
}
