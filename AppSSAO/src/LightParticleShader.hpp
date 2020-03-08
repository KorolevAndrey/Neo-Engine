#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "LightParticleComponent.hpp"

#include "Engine.hpp"

using namespace neo;

class LightParticleShader : public Shader {

    public:

        Texture* fireTexture;

        LightParticleShader(const std::string &vert, const std::string &frag) :
            Shader("LightParticle Shader", vert, frag) {
            fireTexture = Library::loadTexture("f.png");
        }

        virtual void render() override {
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

            loadTexture("fire", *fireTexture);

            /* Render light billboard */
            // TODO : instanced?
            for (auto&& light : Engine::getComponentTuples<LightParticleComponent, SpatialComponent>()) {
                loadUniform("M", light->get<SpatialComponent>()->getModelMatrix());

                /* DRAW */
                Library::getMesh("quad")->draw();
            }

            unbind();
        }
        
        virtual void imguiEditor() override {
        }
};
