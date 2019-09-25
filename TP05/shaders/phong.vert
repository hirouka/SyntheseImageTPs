#version 330

// TODO: add an attribute for coordinates 
// transfer coordinates from vertex to fragment shader via rasterization
// (see exercice 1)

// input attributes 
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 coords;


uniform mat4 mdvMat; // modelview matrix (constant for all the vertices)
uniform mat4 projMat; // projection matrix (constant for all the vertices)
uniform mat3 normalMat; // normal matrix (constant for all the vertices)

// output variables that will be interpolated during rasterization (equivalent to varying)
out vec3 normalView;
out vec3 eyeView;
out vec2 coordsout;
void main() {
  gl_Position = projMat*mdvMat*vec4(position,1.0);
  normalView  = normalize(normalMat*normal);
  eyeView     = normalize((mdvMat*vec4(position,1.0)).xyz);
  coordsout   = coords;
  
}
