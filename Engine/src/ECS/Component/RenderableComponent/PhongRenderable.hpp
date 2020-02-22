#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Material.hpp"

namespace neo {
    class Material;
    class Mesh;
    class Texture;

    namespace renderable {

        class PhongRenderable : public Component {

        public:
            // These will be replaced by renderer handles eventually
            const Texture& mDiffuseMap;
            Material mMaterial;

            PhongRenderable(GameObject *go, const Texture& diffuseMap, Material material = Material{}) :
                Component(go),
                mDiffuseMap(diffuseMap),
                mMaterial(material)
            {}
        };
    }
}
