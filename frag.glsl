uniform float u_time;

float srand(float id, float seed) {
    return fract(sin(dot(vec2(id, seed), 
    vec2(127.1, 311.7))) * 43758.5453);
}

float rand(float id) { return srand(id, 0.);} 

mat2 rot(float a)
{
    return mat2(cos(a),-sin(a),sin(a),cos(a));
}

float disc( vec2 p, float r )
{
    return length(p) - r;
}

float card( in vec2 p, in vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) +
    min(max(d.x,d.y),0.0);
}

vec4 draw(vec4 c) { return vec4(1.);}

uniform float u_mx;

void main() {

    vec2 st = gl_FragCoord.xy/222.;
   gl_FragColor = vec4(u_mx/222., sin(u_time) ,
                       disc(st, .1)*1.,
                       1.0);
}