#include "phong.glsl"
#include "alphaDiscard.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;
in vec3 fragTan;
in vec3 fragBitan;
in mat3 TBN;

uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shine;
uniform sampler2D alphaMap;
uniform sampler2D ambientMap;
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

out vec4 color;

void main() {
    float alpha = texture(alphaMap, fragTex).r;
    alphaDiscard(alpha);

    vec3 amb = ambient * texture(ambientMap, fragTex).rgb;
    vec3 dif = diffuse * texture(diffuseMap, fragTex).rgb;
    vec3 spec = specular + texture(specularMap, fragTex).rgb;
    vec3 V = (camPos - fragPos.xyz);
    vec3 L = (lightPos - fragPos.xyz);
    vec3 n = normalize(fragNor);
    vec3 phong = getPhong(n, V, L, lightAtt, lightCol, dif, spec, shine);

    color = vec4(phong, alpha);
}