#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Engine.hpp"

#include "SunComponent.hpp"
#include "SunOccluderComponent.hpp"

using namespace neo;

class GodRayOccluderShader : public Shader {

    public:

        GodRayOccluderShader(const std::string &vert, const std::string &frag) :
            Shader("GodRayOccluder Shader", vert, frag) {
        }

        virtual void render() override {
            auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>();
            if (!camera) {
                return;
            }

            auto fbo = Library::getFBO("godray");
            fbo->bind();
            glm::ivec2 frameSize = Window::getFrameSize() / 2;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            /* Load PV */
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            for (auto& renderableIt : Engine::getComponentTuples<SunOccluderComponent, SpatialComponent>()) {
                auto renderable = renderableIt->get<SunOccluderComponent>();
                auto renderableSpatial = renderableIt->get<SpatialComponent>();

                loadUniform("M", renderableSpatial->getModelMatrix());

                loadTexture("alphaMap", *renderable->alphaMap);

                renderable->mesh->draw();
            }
        }
};
