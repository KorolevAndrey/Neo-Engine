#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

class GBufferComponent: public Component {
public:
    Material mMaterial;
    const Texture& mDiffuseMap;
    const Texture& mNormalMap;

    GBufferComponent(GameObject *go, const Texture& diffuseMap, const Texture& normalMap, Material material) :
        Component(go),
        mDiffuseMap(diffuseMap),
        mNormalMap(normalMap),
        mMaterial(material)
    {}
};

