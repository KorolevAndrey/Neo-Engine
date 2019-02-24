#version 330 core

in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

uniform sampler2D lightOutput;
uniform sampler2D gDiffuse;

uniform float diffuseAmount;

out vec4 color;

void main() {
    vec4 lightOutput = texture(lightOutput, fragTex);
    vec4 diffuse = texture(gDiffuse, fragTex);
    color.rgb = diffuseAmount * diffuse.rgb + lightOutput.rgb;
    color.a = 1.f;
}