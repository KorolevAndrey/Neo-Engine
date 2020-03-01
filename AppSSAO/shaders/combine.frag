
in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

uniform sampler2D lightOutput;

uniform bool showAO;

out vec4 color;

void main() {
    vec4 lightOutput = texture(lightOutput, fragTex);
    vec4 aoOutput = texture(inputFBO, fragTex);
    color.rgb = lightOutput.rgb * (showAO ? aoOutput.r : 1.f);
    color.a = 1.f;
}