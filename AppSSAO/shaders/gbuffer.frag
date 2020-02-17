
in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;
in vec3 fragTan;
in vec3 fragBitan;

uniform sampler2D diffuseMap;

uniform vec3 diffuseColor;
uniform vec3 ambientColor;

uniform bool useNormalMap;
uniform sampler2D normalMap;

layout (location = 0) out vec4 gDiffuse;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gTan;
layout (location = 3) out vec4 gBitan;

void main() {
    gDiffuse = vec4(useDiffuseMap ? texture(diffuseMap, fragTex).rgb : diffuseMaterial, 1.f);
    gNormal  = vec4(0,1,0,1); //vec4(useNormalMap ? normalize(texture(normalMap, fragTex).rgb) : normalize(fragNor), 1.f);
    gTan     = vec4(normalize(fragTan), 1.f);
    gBitan   = vec4(normalize(fragBitan), 1.f);
}  
