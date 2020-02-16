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

        for (auto& renderable : Engine::getComponents<renderable::PhongRenderable>()) {
            if (auto renderableSpatial = renderable->getGameObject().getComponentByType<SpatialComponent>()) {

                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = renderable->getGameObject().getComponentByType<BoundingBoxComponent>()) {
                        float radius = glm::max(glm::max(renderableSpatial->getScale().x, renderableSpatial->getScale().y), renderableSpatial->getScale().z) * boundingBox->getRadius();
                        if (!cameraFrustum->isInFrustum(renderableSpatial->getPosition(), radius)) {
                            continue;
                        }
                    }
                }

                loadUniform("M", renderableSpatial->getModelMatrix());
                loadUniform("N", renderableSpatial->getNormalMatrix());

                loadUniform("ambient", renderable->material.ambient);
                loadUniform("diffuse", renderable->material.diffuse);
                loadUniform("specular", renderable->material.specular);
                loadUniform("shine", renderable->material.shininess);

                loadTexture("ambientMap", *renderable->ambientMap);
                loadTexture("diffuseMap", *renderable->diffuseMap);
                loadTexture("specularMap", *renderable->specularMap);
                loadTexture("normalMap", *renderable->normalMap);

                renderable->mesh->draw();
            }
        }
    }

};
