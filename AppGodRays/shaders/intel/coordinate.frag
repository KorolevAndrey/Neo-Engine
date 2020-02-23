#include "postprocess.glsl"

uniform sampler2D linearz;

out vec4 color;

void main() {

    float d = texture(linearz, fragTex).r;
    color = vec4(1,0,0,1);

    // Compute direction of the ray going from the light through the pixel
    float2 f2RayDir = normalize( In.m_f2PosPS - g_LightAttribs.f4LightScreenPos.xy );

    // Computing epipolar line exit point
    // uint uiBoundary = clamp(floor( fEpipolarSlice * 4 ), 0, 3); 
    // float fPosOnBoundary = frac( fEpipolarSlice * 4 ); 
    // float fBoundaryCoord = -1 + 2*fPosOnBoundary; 
    // Left Bttom Right Top 
    // float4 f4BoundaryXCoord = float4( -1, fBoundaryCoord, 1, -fBoundaryCoord); 
    // float4 f4BoundaryYCoord = float4(-fBoundaryCoord, -1, fBoundaryCoord, 1); 
    // bool4 b4BoundaryFlags = bool4(uiBoundary.xxxx == uint4(0,1,2,3)); 
    // Select the right coordinates for the boundary 
    //float2 f2ExitPoint = float2(dot(f4BoundaryXCoord, b4BoundaryFlags), dot(f4BoundaryYCoord, b4BoundaryFlags));

    // TODO : Computing epipolar line entry point given its exit point
    // TODO : Rescaling epipolar line length to provide even texel to screen pixel correspondence
    // TODO : Computing camera space z for the location
}