#version 330

in  vec3  normalView;
in  vec3  eyeView;
in  vec2  uvcoord;
in  float depth;

// the 2 output locations: refers to glDrawBuffers in the cpp file 
layout(location = 0) out vec4 outBuffer1;
layout(location = 1) out vec4 outBuffer2;

uniform vec3 color;

void main() {
  vec3 n = normalize(normalView);

  outBuffer1 = vec4(color,1);
  outBuffer2 = vec4(n,depth); //AJOUTE : On remplace le 1 par depth (on profite de cette place libre)
  //NB : On aurait pu mettre depth dans outBuffer1, ça marche aussi.
  
}
