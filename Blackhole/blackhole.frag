#version 430

uniform vec3 cameraPos;
uniform float yaw;
uniform float pitch;
uniform vec3 blackholePos;
uniform float blackholeRadius;
uniform float blackholeMass;
uniform float rayStepSize;
uniform int rayIterations;
uniform float screenWidth;
uniform float screenHeight;
uniform float fov; // in degrees

out vec4 fragColor;

void main() {
    // Normalized device coordinates [-1, 1]
    vec2 uv = (gl_FragCoord.xy / vec2(screenWidth, screenHeight)) * 2.0 - 1.0;
    uv.x *= screenWidth / screenHeight; // preserve aspect ratio

    // Convert FOV to radians
    float fovRad = radians(fov);
    
    // Scale uv by tangent of half FOV
    uv *= tan(fovRad / 2.0);

    // Camera angles in radians
    float yawRad = radians(yaw);
    float pitchRad = radians(pitch);

    // Convert spherical coordinates to Cartesian for ray direction
    vec3 forward;
    forward.x = cos(pitchRad) * sin(yawRad);
    forward.y = sin(pitchRad);
    forward.z = cos(pitchRad) * cos(yawRad);

    // Right and up vectors
    vec3 right = normalize(vec3(sin(yawRad - 3.14159/2.0), 0.0, cos(yawRad - 3.14159/2.0)));
    vec3 up = cross(right, forward);

    // Final ray direction
    vec3 rayDir = normalize(forward + uv.x * right + uv.y * up);
    vec3 rayPos = cameraPos;

    for (int i = 0; i < rayIterations; i++) {
        vec3 dirToBH = blackholePos - rayPos;
        float dist = length(dirToBH);

        vec3 spherePos = vec3(0.0, 2.0, 5.0);
        vec3 dirToSphere = spherePos - rayPos;
        float distToSphere = length(dirToSphere);

        if (dist < blackholeRadius) {
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }

        if (distToSphere < 2.0) {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
            return;
        }

        if (abs(rayPos.y - blackholePos.y) < 0.15 && dist > blackholeRadius && dist < 5.0) {
            fragColor = vec4(rayDir * 0.5 + 0.5, 1.0);
            return;
        }

        vec3 attraction = (blackholeMass / (dist * dist)) * dirToBH;
        rayDir += attraction;
        rayDir = normalize(rayDir);
        rayPos += rayDir * rayStepSize;
    }

    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
