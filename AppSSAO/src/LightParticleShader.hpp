#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "LightParticleComponent.hpp"

#include "Engine.hpp"

using namespace neo;

class LightParticleShader : public Shader {

    public:

        LightParticleShader(const std::string &vert, const std::string &frag) :
            Shader("LightParticle Shader", vert, frag) {
        }

        virtual void render() override {
            auto mainCamera = Engine::getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>();
            if (!mainCamera) {
                return;
            }

            auto fbo = Library::getFBO("lightpass");
            fbo->bind();
            CHECK_GL(glViewport(0, 0, fbo->mTextures[0]->mWidth, fbo->mTextures[0]->mHeight));

            bind();

            if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());
                glm::mat4 Vi = camera->get<CameraComponent>()->getView();
                Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
                Vi = glm::transpose(Vi);
                loadUniform("Vi", Vi);
            }

            /* Render light billboard */
            // TODO : instanced?
            for (auto&& light : Engine::getComponentTuples<LightParticleComponent, SpatialComponent>()) {
                loadUniform("M", light->get<SpatialComponent>()->getModelMatrix());
                loadUniform("center", light->get<SpatialComponent>()->getPosition());

                /* DRAW */
                Library::getMesh("quad")->draw();
            }

            unbind();
        }
        
        virtual void imguiEditor() override {
        }
};
