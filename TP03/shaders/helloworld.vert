#version 330

// starting from OpenGL 3.3 it is possible to use this syntax to make the relations with arrays
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 mvp; // modelview projection matrix (constant for all the vertices)
uniform float time; // AJOUT : ajout du temps 

out vec3 rasterizedNormal; // normal (will be rasterized from vertex to fragment shader)

void main() {
vec3 pos = position; //AJOUT : Car position est protégé en écriture
//-------------------------------------
// AJOUT / A COMPLETER : Ici qu'on fait les modif !
//pos.x = pos.x*0.01;
//pos.x = pos.x + 0.2*sin(time*10 + pos.y*10); //0.2 pout l'amplitude et 10 pour la vitesse

pos.z = pos.z+0.1*sin(time*40);

if(pos.z < -0.15){
  pos.y = pos.y+0.03*sin(time*20);
}
pos.y = pos.y-tan(time);
//-------------------------------------
gl_Position = mvp*vec4(pos,1.0);
 
  
  rasterizedNormal = normal;
}
