#version 430

uniform vec3 cameraPos;
uniform float yaw;
uniform float pitch;
uniform vec3 blackholePos;
uniform float blackholeRadius;
uniform float blackholeMass;
uniform int rayIterations;
uniform float screenWidth;
uniform float screenHeight;
uniform float fov;
uniform float accretionDiskRadius;

out vec4 fragColor;
vec3 repeat(vec3 p, float period) {
    return mod(p + 0.5 * period, period) - 0.5 * period;
}

float sdSphere(vec3 p, float r) {
    // p = repeat(p, 30);
    return length(p) - r;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdBoxFrame( vec3 p, vec3 b, float e )
{
    // p = repeat(p, 30);
    p = abs(p  )-b;
    vec3 q = abs(p+e)-e;
    return min(min(
    length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
    length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
    length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}

float sdTorus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float sdCappedCylinder( vec3 p, float r, float h )
{
  vec2 d = abs(vec2(length(p.xz),p.y)) - vec2(r,h);
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

void main() {
    // Normalized device coordinates [-1, 1]
    vec2 uv = (gl_FragCoord.xy / vec2(screenWidth, screenHeight)) * 2.0 - 1.0;
    uv.x *= screenWidth / screenHeight; // preserve aspect ratio
    
    // Crosshair
    // float cursorSize = 0.005;
    // if (uv.x < cursorSize && uv.x > -cursorSize && uv.y < cursorSize && uv.y > -cursorSize) {
    //     fragColor = vec4(1.0);
    //     return;
    // }

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

    // Schwarzschild radius
    float rs = 2.0 * blackholeMass;

    float diskThickness = 0.05;

    vec3 targetPosition;

    for (int i = 0; i < rayIterations; i++) {
        // Calculate closest distance
        float blackholeDistance = sdSphere(rayPos - blackholePos, rs);
        // float accretionDiskDistance =  sdCappedCylinder(rayPos - blackholePos, accretionDiskRadius, diskThickness);
        float ballDistance = sdSphere(repeat(rayPos - vec3(25.0), 50.0), 2.0);
        float boxDistance = sdBoxFrame(rayPos - vec3(10, 0, 0), vec3(3.0), 0.3);
        float torusDistance = sdTorus(rayPos - vec3(0.0, -10.0, 0.0), vec2(3.0, 0.5));

        float closestDist = blackholeDistance;
        // closestDist = min(closestDist, accretionDiskDistance);
        closestDist = min(closestDist, ballDistance);
        closestDist = min(closestDist, torusDistance);


        // Vector from black hole to current ray position
        vec3 r = rayPos - blackholePos;
        float rLen = length(r);

        // Avoid division explosion
        float epsilon = 1e-6;
        float safeR = max(rLen, epsilon);

        // Schwarzschild radius
        float rs = 2.0 * blackholeMass;

        // Project rayDir onto r to compute radial component
        float vr = dot(rayDir, r) / safeR;

        // General Relativistic "acceleration" term for null geodesics
        vec3 acc = -(rs / (safeR * safeR)) * 
                ((1.0 - 1.5 * (vr * vr)) * r / safeR);

        // Curvature correction so larger marching steps still bend correctly
        float metricScale = 1.0 / (1.0 + rs / safeR);

        // Apply curvature scaled by affine (not spatial!) step
        rayDir += acc * closestDist;
        rayDir = normalize(rayDir);

        // Move forward by standard spatial marching
        rayPos += rayDir * closestDist;



        float touchDistance = 0.01;
        if (closestDist < touchDistance) {
            if (blackholeDistance < touchDistance  && dot(rayDir, blackholePos - rayPos) < 0.0) {
                fragColor = vec4(0.0, 0.0, 0.0, 1.0);
                return;
            }
            // if (accretionDiskDistance < touchDistance) {
            //     float colorAbberation = 1 + 0.1 * sin((length(rayPos.xz - blackholePos.xz) - accretionDiskRadius)*2.0);
            //     vec3 color = vec3(1.0 * colorAbberation, 0.56 * colorAbberation, 0);
            //     normalize(color);

            //     fragColor = vec4(color, 1.0);
            //     return;
            // }
            if (torusDistance < touchDistance || boxDistance < touchDistance || ballDistance < touchDistance) { 
                fragColor = vec4((rayDir * 0.5 + 0.5), 1.0);
                return;
            }
        }

        if (closestDist > 1000.0 || length(cameraPos - rayPos) > 1000.0) {
            // fragColor = vec4(1 - (rayDir * 0.5 + 0.5), 1.0);
            fragColor = vec4(0.0);
            return;
        }

    //    if (sdBoxFrame(rayPos, vec3(4.0), 0.5) < 0) {
    //        fragColor = vec4(1 - (rayDir * 0.5 + 0.5), 1.0);
    //        return;
    //    }





    }

    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
