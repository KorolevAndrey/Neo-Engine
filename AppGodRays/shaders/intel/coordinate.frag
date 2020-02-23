#include "postprocess.glsl"

uniform sampler2D linearz;

uniform mat4 V;
uniform vec3 lightPos;
uniform vec2 screenSize;

uniform int numSamples;
uniform int numSlices;

out vec4 color;

#define INVALID_EPIPOLAR_LINE vec4(-1000,-1000, -100, -100)
#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

// https://github.com/GameTechDev/CloudySky/blob/7bf3c5210fc1c3d5a042c1731189a817fe7edeed/fx/LightScattering.fx

bvec4 bAnd(bvec4 a, bvec4 b) {
    return bvec4(a.x && b.x, a.y && b.y, a.z && b.z, a.w && b.w);
}

vec4 bNot(bvec4 a) {
    return vec4(!a.x, !a.y, !a.z, !a.w);
}

vec4 GetOutermostScreenPixelCoords() {
    return vec4(-1.0,-1.0,1.0,1.0) + vec4(1.0, 1.0, -1.0, -1.0) / vec4(screenSize.xy, screenSize.xy);
}

vec2 GetEpipolarLineEntryPoint(vec2 f2ExitPoint, vec4 viewLightPos) {
    vec2 f2EntryPoint;

    
    bool bIsLightOnScreen = (abs(viewLightPos.x) <= 1.0 - 1.0/float(screenSize.x)) && (abs(viewLightPos.y) <= 1.0 - 1.0/float(screenSize.y));
    if( bIsLightOnScreen )
    {
        f2EntryPoint = viewLightPos.xy;
    }
    else
    {
        vec2 f2RayDir = f2ExitPoint.xy - viewLightPos.xy;
        float fDistToExitBoundary = length(f2RayDir);
        f2RayDir /= fDistToExitBoundary;
        vec4 f4Boundaries = GetOutermostScreenPixelCoords();
        bvec4 b4IsCorrectIntersectionFlag; 
        b4IsCorrectIntersectionFlag = greaterThan(abs(f2RayDir.xyxy), vec4(1e-5));

        vec4 f4DistToBoundaries = (f4Boundaries - viewLightPos) / (f2RayDir.xyxy + bNot(b4IsCorrectIntersectionFlag));   

        b4IsCorrectIntersectionFlag = bAnd(b4IsCorrectIntersectionFlag, greaterThan(f4DistToBoundaries, vec4(fDistToExitBoundary - 1e-4)));
        f4DistToBoundaries = vec4(b4IsCorrectIntersectionFlag) * f4DistToBoundaries + bNot(b4IsCorrectIntersectionFlag) * vec4(-FLT_MAX);

        float fFirstIntersecDist = 0.0;
        fFirstIntersecDist = max(fFirstIntersecDist, f4DistToBoundaries.x);
        fFirstIntersecDist = max(fFirstIntersecDist, f4DistToBoundaries.y);
        fFirstIntersecDist = max(fFirstIntersecDist, f4DistToBoundaries.z);
        fFirstIntersecDist = max(fFirstIntersecDist, f4DistToBoundaries.w);

        f2EntryPoint = viewLightPos.xy + f2RayDir * fFirstIntersecDist;
    }

    return f2EntryPoint;
}

vec4 getRayEntryAndExit(vec4 viewLightPos) {

    float fEpipolarSlice = clamp(fragTex.x - 0.5 / float(numSamples), 0.0, 1.0);

    float uiBoundary = clamp(floor( fEpipolarSlice * 4.0 ), 0.0, 3.0);
    float fPosOnBoundary = fract( fEpipolarSlice * 4.0 );

    vec4 b4BoundaryFlags = vec4(uiBoundary == 0.0 ? 1.0 : 0.0, uiBoundary == 1.0 ? 1.0 : 0.0, uiBoundary == 2.0 ? 1.0 : 0.0, uiBoundary == 3.0 ? 1.0 : 0.0);

    vec4 f4OutermostScreenPixelCoords = GetOutermostScreenPixelCoords();// xyzw = (left, bottom, right, top)

    vec4 b4IsInvalidBoundary = vec4( (viewLightPos.xyxy - f4OutermostScreenPixelCoords.xyzw) * vec4(1,1,-1,-1));
    b4IsInvalidBoundary.x = b4IsInvalidBoundary.x < 0.0 ? 1.0 : 0.0;
    b4IsInvalidBoundary.y = b4IsInvalidBoundary.y < 0.0 ? 1.0 : 0.0;
    b4IsInvalidBoundary.z = b4IsInvalidBoundary.z < 0.0 ? 1.0 : 0.0;
    b4IsInvalidBoundary.w = b4IsInvalidBoundary.w < 0.0 ? 1.0 : 0.0;
    if( dot(b4IsInvalidBoundary, b4BoundaryFlags) != 0.0) {
        return INVALID_EPIPOLAR_LINE;
    }

    vec4 f4BoundaryXPos = vec4(               0.0, fPosOnBoundary,                1.0, 1.0-fPosOnBoundary);
    vec4 f4BoundaryYPos = vec4( 1.0-fPosOnBoundary,              0.0,  fPosOnBoundary,                1.0);
    vec2 f2ExitPointPosOnBnd = vec2( dot(f4BoundaryXPos, b4BoundaryFlags), dot(f4BoundaryYPos, b4BoundaryFlags) );
    vec2 f2ExitPoint = mix(f4OutermostScreenPixelCoords.xy, f4OutermostScreenPixelCoords.zw, f2ExitPointPosOnBnd);
    vec2 f2EntryPoint = GetEpipolarLineEntryPoint(f2ExitPoint, viewLightPos);

    return vec4(f2EntryPoint, f2ExitPoint);
}

void main() {

    float d = texture(linearz, fragTex).r;
    vec4 viewLightPos = V * vec4(lightPos, 1.0);

    vec4 point = getRayEntryAndExit(viewLightPos);

    vec2 f2EntryPoint = point.xy;
    vec2 f2ExitPoint = point.zw;

    if( abs(f2EntryPoint.x) > 1+1e-4 
    || abs(f2EntryPoint.y) > 1+1e-4 ) 
    discard; 

    float fEpipolarSliceScreenLen = length( (f2ExitPoint - f2EntryPoint) * screenSize / 2.0 ); 
    f2ExitPoint = f2EntryPoint + (f2ExitPoint - f2EntryPoint) * max((1.0 / screenSize.x) / fEpipolarSliceScreenLen, 1.0);

    float fSamplePosOnEpipolarLine = fragTex.x - 0.5 / float(numSlices);
    fSamplePosOnEpipolarLine *= float(numSlices) / (float(numSlices)-1.0);
    fSamplePosOnEpipolarLine = clamp(fSamplePosOnEpipolarLine, 0.0, 1.0);

    vec2 f2XY = mix(f2EntryPoint, f2ExitPoint, fSamplePosOnEpipolarLine); 
    if( abs(f2XY.x) > 1+1e-4 
    || abs(f2XY.y) > 1+1e-4) 
    discard;


    color = vec4(f2EntryPoint, f2ExitPoint);

}