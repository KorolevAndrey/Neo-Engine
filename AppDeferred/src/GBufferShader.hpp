#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"
#include "Renderer/Renderer.hpp"
#include "Window/Window.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

class GBufferShader : public Shader {

    public:

        GBufferShader(const std::string &vert, const std::string &frag) :
            Shader("GBufferShader", vert, frag) {

            // Create gbuffer 
            auto gbuffer = Library::getFBO("gbuffer");
            gbuffer->generate();

            // Format for color buffers
            TextureFormat format = { GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT };
            gbuffer->attachColorTexture(Window::getFrameSize(), 4, format); // color
            gbuffer->attachColorTexture(Window::getFrameSize(), 4, format); // diffuse
            gbuffer->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_REPEAT); // depth
            gbuffer->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("gbuffer")->resize(frameSize);
            });
        }

        virtual void render(const CameraComponent &camera) override {
            auto fbo = Library::getFBO("gbuffer");
            fbo->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            bind();
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (auto& model : Engine::getComponents<MeshComponent>()) {
                loadUniform("M", model->getGameObject().getSpatial()->getModelMatrix());

                /* Bind mesh */
                const Mesh & mesh(model->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                /* Bind diffuse map or material */
                auto matComp = model->getGameObject().getComponentByType<MaterialComponent>();
                if (matComp) {
                    loadUniform("ambient", matComp->mAmbient);
                }
                if (auto diffMap = model->getGameObject().getComponentByType<DiffuseMapComponent>()) {
                    diffMap->mTexture->bind();
                    loadUniform("useDiffuseMap", true);
                    loadUniform("diffuseMap", diffMap->mTexture->mTextureID);
                }
                else {
                    loadUniform("useDiffuseMap", false);
                    if (matComp) {
                        loadUniform("diffuseMaterial", matComp->mDiffuse);
                    }
                }

                /* Bind normal map */
                auto normalMap = model->getGameObject().getComponentByType<neo::NormalMapComponent>();
                if (normalMap) {
                    normalMap->mTexture->bind();
                    loadUniform("useNormalMap", true);
                    loadUniform("normalMap", normalMap->mTexture->mTextureID);
                }
                else {
                    loadUniform("useNormalMap", false);
                    loadUniform("N", model->getGameObject().getSpatial()->getNormalMatrix());
                }

                /* DRAW */
                mesh.draw();
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            unbind();
    }
};