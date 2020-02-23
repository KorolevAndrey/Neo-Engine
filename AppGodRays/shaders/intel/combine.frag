#include "postprocess.glsl"

uniform sampler2D linearz;
uniform sampler2D coordinate;

out vec4 color;

void main() {

    vec4 d = texture(coordinate, fragTex);
    color = vec4(d.rgb,1);
}