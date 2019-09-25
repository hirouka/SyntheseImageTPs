#version 330

out vec4 bufferColor;

in vec3 rasterizedNormal; // received (rasterized) normal

uniform vec3 myColor;
uniform float time; // AJOUT : ajout du temps 

void main() {
  // re-normalize 
  vec3 normal = normalize(rasterizedNormal);
  
  //-------------------------------------
// AJOUT / A COMPLETER : Ici qu'on fait les modif !

  // normal coordinates are used as colors here 
  //bufferColor = vec4(normal*0.5+0.5,1.0);

  // color modified by a global variable 
  //bufferColor = vec4(myColor*(normal*0.5+0.5),1.0);
  //bufferColor = vec4(myColor*(sin(time*10)*0.5+0.5)*(normal*0.5+0.5),1.0);
  bufferColor = vec4(cos(time*10)*normal.x*0.5+0.5, sin(time*10)*normal.y*0.5+0.5, tan(time*10)*normal.z*0.5+0.5, 1.0);

  //-------------------------------------
}
