#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Material.hpp"

using namespace neo;

class MyPhongRenderable : public Component {
public:
    // These will be replaced by renderer handles eventually
    const Texture& mAmbientMap;
    const Texture& mDiffuseMap;
    const Texture& mNormalMap;
    const Texture& mSpecularMap;
    Material mMaterial;

    MyPhongRenderable(GameObject *go, const Texture& ambientMap, const Texture& diffuseMap, const Texture& normalMap, const Texture& specularMap, Material material = Material{}) :
        Component(go),
        mAmbientMap(diffuseMap),
        mDiffuseMap(diffuseMap),
        mNormalMap(diffuseMap),
        mSpecularMap(diffuseMap),
        mMaterial(material)
    {}
};
