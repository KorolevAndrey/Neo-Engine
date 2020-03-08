
in vec2 fragTex;

uniform sampler2D fire;

out vec4 color;

void main() {
    vec4 s = texture(fire, fragTex);
    color = vec4(s.rgb, (s.r + s.g + s.b) / 3.0);
}