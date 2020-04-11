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

            auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>();
            NEO_ASSERT(camera, "No camera!");
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());
            glm::mat4 Vi = camera->get<CameraComponent>()->getView();
            Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
            Vi = glm::transpose(Vi);
            loadUniform("Vi", Vi);


            // sort light indices -- component tuples aren't copyable 
            auto&& lights = Engine::getComponentTuples<LightParticleComponent, SpatialComponent>();

            /* Render light billboard */
            // TODO : instanced?
            for (auto&& light : Engine::getComponentTuples<LightParticleComponent, SpatialComponent>()) {
                
                // TODO VFC

                loadUniform("M", light->get<SpatialComponent>()->getModelMatrix());

                loadUniform("lightCol", light->get<LightParticleComponent>()->base.color);

                /* DRAW */
                Library::getMesh("quad")->draw();
            }

            unbind();
        }
        
        virtual void imguiEditor() override {
        }
};
