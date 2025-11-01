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
uniform float accretionDiskRadius;

out vec4 fragColor;
vec3 repeat(vec3 p, float period) {
    return mod(p + 0.5 * period, period) - 0.5 * period;
}

float sdSphere(vec3 p, float r) {
    p = repeat(p, 30);
    return length(p) - r;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdBoxFrame( vec3 p, vec3 b, float e )
{
    p = repeat(p, 30);
    p = abs(p  )-b;
    vec3 q = abs(p+e)-e;
    return min(min(
    length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
    length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
    length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}

void main() {
    // Normalized device coordinates [-1, 1]
    vec2 uv = (gl_FragCoord.xy / vec2(screenWidth, screenHeight)) * 2.0 - 1.0;
    uv.x *= screenWidth / screenHeight; // preserve aspect ratio

    float cursorSize = 0.01;
    if (uv.x < cursorSize && uv.x > -cursorSize && uv.y < cursorSize && uv.y > -cursorSize) {
        fragColor = vec4(1.0);
        return;
    }

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

        if (dist < blackholeRadius  && dot(normalize(rayDir), normalize(dirToBH)) > 0.0) {
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }

        if (sdSphere(rayPos, 2.0) < 0.0) {
            fragColor = vec4(1 - (rayDir * 0.5 + 0.5), 1.0);
            return;
        }

//        if (sdBoxFrame(rayPos, vec3(2.0), 0.1) < 0) {
//            fragColor = vec4(1 - (rayDir * 0.5 + 0.5), 1.0);
//            return;
//        }

        if (abs(rayPos.y - blackholePos.y) < 0.15 && dist > blackholeRadius && dist < accretionDiskRadius) {
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
