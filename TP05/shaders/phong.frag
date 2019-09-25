#version 330

// TODO: displayed rasterized texture coordinates (exercice 1)
// TODO: declare a uniform texture (uniform sampler2D) and make the link in the CPU side (exercice 3)
// TODO: use texture coordinates and texture to apply modify the shading (exercice 3)


in  vec3 normalView;
in  vec3 eyeView;
in  vec2 coordsout;

out vec4 bufferColor;

uniform vec3 light;
uniform sampler2D simple;

void main() {
  // Phong parameters (colors and roughness)
  const vec3 ambient  = vec3(0.7,0.3,0.0);
  const vec3 diffuse  = vec3(0.3,0.4,0.0);
  const vec3 specular = vec3(0.8,0.2,0.2);
  const float et = 10.0;

  // normal / view and light directions (in camera space)
  vec3 n = normalize(normalView);
  vec3 e = normalize(eyeView);
  vec3 l = normalize(light);

  // diffuse and specular components of the phong shading model
  float diff = max(dot(l,n),0.0);
  float spec = pow(max(dot(reflect(l,n),e),0.0),et);

  // final color 
  vec3 color = ambient + diff*diffuse + spec*specular;
  //bufferColor = texture(simple,coordsout*2);
  vec4 textureN = texture(simple,coordsout);
  bufferColor =  (ambient,0.0) + diff*textureN*textureN + (spec*specular,0.0);

}

