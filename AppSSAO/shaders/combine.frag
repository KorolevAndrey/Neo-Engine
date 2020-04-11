
in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

uniform sampler2D lightOutput;

out vec4 color;

void main() {
    vec4 lightOutput = texture(lightOutput, fragTex);
    vec4 aoOutput = texture(inputFBO, fragTex);
    color.rgb = lightOutput.rgb * aoOutput.r;
    color.a = 1.f;
}