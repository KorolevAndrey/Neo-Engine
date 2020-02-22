#pragma once

#include "Engine.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

using namespace neo;

class MyPhongShader : public Shader {

public:

    MyPhongShader(const std::string& vert, const std::string& frag) :
        Shader("MyPhong Shader", vert, frag)
    { }

    virtual void render() override {
        bind();

        /* Load PV */
        auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>();
        NEO_ASSERT(camera, "No main camera exists");
        loadUniform("P", camera->get<CameraComponent>()->getProj());
        loadUniform("V", camera->get<CameraComponent>()->getView());

        loadUniform("camPos", camera->get<SpatialComponent>()->getPosition());

        /* Load light */
        if (auto light = Engine::getComponentTuple<LightComponent, SpatialComponent>()) {
            loadUniform("lightPos", light->get<SpatialComponent>()->getPosition());
            loadUniform("lightCol", light->get<LightComponent>()->mColor);
            loadUniform("lightAtt", light->get<LightComponent>()->mAttenuation);
        }

        const auto& cameraFrustum = camera->mGameObject.getComponentByType<FrustumComponent>();

        for (auto& renderableIt : Engine::getComponentTuples<MyPhongRenderable, MeshComponent, SpatialComponent>()) {
            auto spatial = renderableIt->get<SpatialComponent>();

            // VFC
            if (cameraFrustum) {
                MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                if (const auto& boundingBox = renderableIt->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                    float radius = glm::max(glm::max(spatial->getScale().x, spatial->getScale().y), spatial->getScale().z) * boundingBox->getRadius();
                    if (!cameraFrustum->isInFrustum(spatial->getPosition(), radius)) {
                        continue;
                    }
                }
            }

            loadUniform("M", spatial->getModelMatrix());
            loadUniform("N", spatial->getNormalMatrix());

            auto renderable = renderableIt->get<MyPhongRenderable>();
            loadUniform("ambient", renderable->mMaterial.mAmbient);
            loadUniform("diffuse", renderable->mMaterial.mDiffuse);
            loadUniform("specular", renderable->mMaterial.mSpecular);
            loadUniform("shine", renderable->mMaterial.mShininess);
            loadTexture("alphaMap", renderable->mAlphaMap);
            loadTexture("ambientMap", renderable->mAmbientMap);
            loadTexture("diffuseMap", renderable->mDiffuseMap);
            loadTexture("specularMap", renderable->mSpecularMap);
            loadTexture("normalMap", renderable->mNormalMap);

            renderableIt->get<MeshComponent>()->mMesh.draw();
        }
    }

};
