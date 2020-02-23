#include "postprocess.glsl"

uniform sampler2D linearz;

out vec4 color;

void main() {

    float d = texture(linearz, fragTex).r;
    color = vec4(d,0,0,1);
}