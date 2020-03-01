#include "alphaDiscard.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform sampler2D alphaMap;
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;

layout (location = 0) out vec4 gDiffuse;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gSpecular;

void main() {
    alphaDiscard(texture(alphaMap, fragTex).r);

    gDiffuse = vec4(texture(diffuseMap, fragTex).rgb, ambientColor.r);
    gNormal = vec4(normalize(fragNor) * 0.5 + 0.5, 1.f);
    gSpecular = vec4(texture(specularMap, fragTex).r * specularColor, shininess);
}  