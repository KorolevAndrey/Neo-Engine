
in vec2 fragTex;

uniform vec3 center;

out vec4 color;

void main() {
    color = vec4(fragTex, 0.0, 1.0);
}