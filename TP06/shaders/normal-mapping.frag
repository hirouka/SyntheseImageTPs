#version 330

in  vec3 normalView;
in  vec3 tangentView;
in  vec3 eyeView;
in  vec2 uvcoord;

out vec4 bufferColor;

uniform vec3      light;
uniform sampler2D colormap;
uniform sampler2D Aomap;
uniform sampler2D normalMap;

vec3 getModifiedNormal(){
    vec3 n = normalize(normalView);
    vec3 t = normalize(tangentView);
    vec3 b = normalize(cross(n,t));
    mat3 tbn = mat3(t,b,n);
    vec3 tn = normalize(texture(normalMap,uvcoord).xyz*2.0-1);
    return tbn*tn;
}

void main() {
  float et      = 10.0;
  vec4 texColor = texture(colormap,uvcoord);
  vec4 texColor0 = texture(Aomap,uvcoord);
  vec4 texColor1 = texture(normalMap,uvcoord);
  //vec3 n = normalize(normalView);
  vec3 e = normalize(eyeView);
  vec3 l = normalize(light);

  vec3 n = getModifiedNormal();

  float diff = max(dot(l,n),0.);
  float spec = pow(max(dot(reflect(l,n),e),0.0),et);

  bufferColor = texColor0*texColor*(diff + spec)*2.0;
}
