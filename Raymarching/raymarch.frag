eeeeeee#version 330 core
out vec4 FragColor;

uniform vec2 iResolution;
uniform float iTime;
uniform vec3 camPos;
uniform float camYaw;
uniform float camPitch;

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
  return length(max(q,0.0)) + min(max(q.x* q.y,min(q.y*q.z,q.z*q.x)),0.0);
}
float sdRoundBox( vec3 p, vec3 b, float r )
{
  vec3 q = abs(p) - b + r;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
}

float sdCustom(vec3 p) {
    float dstA = dot(sin(p * 10.0), vec3(1.0));
    float dstB = dot(mod(p, 10.0), vec3(1.0));
    return mix(dstA, dstB, iTime);
}


float sdBoxFrame( vec3 p, vec3 b, float e )
{
    p = repeat(p, 10.0);
    p = abs(p)-b;
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

float map(vec3 p) {
    float box = sdBox(p, vec3(5.0));
    float sphere = sdSphere(p, 1.0);
    return min(box, sphere);
}

mat3 getCamera(float yaw, float pitch) {
    float cy = cos(radians(yaw));
    float sy = sin(radians(yaw));
    float cp = cos(radians(pitch));
    float sp = sin(radians(pitch));

    vec3 forward = vec3(cy*cp, sp, sy*cp);
    vec3 right   = normalize(vec3(forward.z, 0.0, -forward.x));
    vec3 up      = cross(right, forward);

    return mat3(right, up, forward);
}

void main() {
    vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;
    uv.x *= iResolution.x / iResolution.y;

    mat3 camMat = getCamera(camYaw, camPitch);
    vec3 ro = camPos;
    vec3 rd = normalize(camMat * vec3(uv, -1.0));

    float minT = 10000.0;
    float t = 0.0;
    for (int i=0; i<500; i++) {
        vec3 pos = ro + rd*t;
        float d = map(pos);
        float distance = length(pos - ro);
        if (minT > d) {
            minT = d;
        }
        if (d < 0.001) {
            FragColor = vec4(1 - distance / 10000);
            break;
        };
        if (t > 10000.0) {
            FragColor = vec4(0.0);
            break;
        };
        t += d;
    }
}
