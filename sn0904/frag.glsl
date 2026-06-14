uniform float u_time;

float srand(float id, float seed) {
    return fract(sin(dot(vec2(id, seed), 
    vec2(127.1, 311.7))) * 43758.5453);
}
float rand(float id) { 
return srand(id, 44.);
} 


float disc( vec2 p, float r )
{
    return r*.1/length(p);
}

uniform float u_mx;
uniform float u_sx;
uniform float u_sy;

void main() {

    vec2 st =  gl_FragCoord;
    st *= u_sy / u_sx;
            vec2 m = u_mx;
    m *= u_sy / u_sx;
    
          float v;
  for(float i=0.; i<41.; i++){
      v += disc(vec2(
      rand(i)*432.,
      rand(i+100.)*u_sy)-st,
      11.+rand(i+u_sy)*8.);
  }

    v += disc(st-134., -24.);
    
   if (v>.7) {v=1.;}
   if (v<.7) {v/=3.;}
   if (v<.15) {v=0.;}

   gl_FragColor = vec4(v*m.x/111., sin(u_time)/3.,
                       disc(st, 1111.1)*1.*v,
                       1.0);
}