
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in vec3 vertTan;
layout(location = 4) in vec3 vertBitan;

uniform mat4 P, V, M;
uniform mat3 N;

out vec4 fragPos;
out vec3 fragNor;
out vec2 fragTex;
out vec3 fragTan;
out vec3 fragBitan;

void main() {
    fragPos = M * vec4(vertPos, 1.0);
    fragTex = vertTex;
    fragNor = normalize(N * normalize(vertNor));
    fragTan = normalize(N * normalize(vertTan));
    fragBitan = normalize(N * normalize(vertBitan));

    gl_Position = P * V * fragPos;
}