#version 330

out vec4 bufferColor;

uniform vec3      light;
uniform sampler2D normalmap;
uniform sampler2D colormap;
uniform float anim;

in vec2 texcoord;

vec4 shade(in vec2 coord) {
  vec4  nd = texture(normalmap,coord,0);
  vec4  c  = texture(colormap ,coord,0);
  
  //c.x = c.x + sin(c.y*10+anim)*0.1; //A REVOIR !
  //c.y = c.y + sin(c.x*10+anim)*0.1;
  
  vec3  n = nd.xyz;
  float d = nd.w; //Profondeur

  vec3 e = vec3(0,0,-1);
  vec3 l = normalize(light);

  float diff = max(dot(l,n),0.0);
  float spec = pow(max(dot(reflect(l,n),e),0.0),d*10.0);

  vec4 S = vec4(c.xyz*(diff+spec),1);   //AJOUTE : S est pour "shade" et prend en compte l'ombrage
  vec4 F = vec4(vec3(0.,0.1,0.1),1); // AJOUTE : F pour "Fog", et donne la couleur du brouillard

  return mix(S,F,clamp(0.,1.,d*2)); //AJOUTE : On mix les deux filtres, "clamp" permet de mettre d (la profondeur) entre 0 et 1
}

void main() {
  vec4 color = shade(texcoord);
  //vec4 col = vec4(clamp(anim, 0.0f, 1.0f),0.,0., 1.);

  //------------------EFFECT-------------------
    


   //-------------------------------------------
  
  bufferColor = vec4(color);
}
