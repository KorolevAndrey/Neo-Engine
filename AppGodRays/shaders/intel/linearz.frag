#include "postprocess.glsl"

uniform float zNear;
uniform float zFar;

out vec4 color;

void main() {

    float depth = texture(inputDepth, fragTex).r;
    float z_n = 2.0 * depth - 1.0;
    float z = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
    color = vec4(z / zFar, 0.0, 0.0, 1.0);
}