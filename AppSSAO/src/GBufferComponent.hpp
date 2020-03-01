#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

class GBufferComponent: public Component {
public:
    const Texture& mAlphaMap;
    const Texture& mDiffuseMap;
    const Texture& mSpecularMap;
    Material mMaterial;

    GBufferComponent(GameObject *go, const Texture& alphaMap, const Texture& diffuseMap, const Texture& specularMap, Material& material) :
        Component(go),
        mAlphaMap(alphaMap),
        mDiffuseMap(diffuseMap),
        mSpecularMap(specularMap),
        mMaterial(material)
    {}

};