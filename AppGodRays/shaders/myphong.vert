layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in vec4 vertTan;

uniform mat4 P, V, M;
uniform mat3 N;

out vec4 fragPos;
out vec3 fragNor;
out vec2 fragTex;
out mat3 fragTBN;
out vec3 fragTan;
out vec3 fragBiTan;

void main() {
    fragPos = M * vec4(vertPos, 1.0);
    fragNor = N * normalize(vertNor);
    fragTex = vertTex;

    vec3 T = normalize(N * vertTan.xyz);
    vec3 Ni = normalize(N * vertNor);
    T = normalize(T - dot(T, Ni) * Ni);
    vec3 B = cross(Ni, T);
    mat3 TBN = mat3(T, B, Ni);

    fragTBN = TBN;
    fragTan = T;
    fragBiTan = B;

    gl_Position = P * V * fragPos;
}