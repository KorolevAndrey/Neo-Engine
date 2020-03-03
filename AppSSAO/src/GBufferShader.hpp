#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Window/Window.hpp"

#include "GBufferComponent.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

class GBufferShader : public Shader {

    public:

        GBufferShader(const std::string &vert, const std::string &frag) :
            Shader("GBuffer Shader", vert, frag) {

            // Create gbuffer 
            auto gbuffer = Library::createFBO("gbuffer");

            TextureFormat format{ GL_RGBA, GL_RGBA, GL_NEAREST, GL_CLAMP_TO_EDGE };
            gbuffer->attachColorTexture(Window::getFrameSize(), format); // diffuse, ambient
            format.mSizedFormat = GL_RGB16F;
            format.mBaseFormat = GL_RGB;
            gbuffer->attachColorTexture(Window::getFrameSize(), format); // normal
            format.mSizedFormat = GL_RGBA16F;
            format.mBaseFormat = GL_RGBA;
            gbuffer->attachColorTexture(Window::getFrameSize(), format); // specular
            gbuffer->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_CLAMP_TO_EDGE);  // depth
            gbuffer->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("gbuffer")->resize(frameSize);
            });
        }

        virtual void render() override {
            auto fbo = Library::getFBO("gbuffer");
            fbo->bind();
            CHECK_GL(glViewport(0, 0, fbo->mTextures[0]->mWidth, fbo->mTextures[0]->mHeight));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            CHECK_GL(glDisable(GL_BLEND));

            bind();

            FrustumComponent* cameraFrustum = nullptr;
            if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());
                cameraFrustum = camera->mGameObject.getComponentByType<FrustumComponent>();
            }

            for (auto& renderableIt : Engine::getComponentTuples<GBufferComponent, MeshComponent, SpatialComponent>()) {
                auto renderable = renderableIt->get<GBufferComponent>();
                auto spatial = renderableIt->get<SpatialComponent>();

                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = renderableIt->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                        if (!cameraFrustum->isInFrustum(spatial->getPosition(), spatial->getScale(), boundingBox->mMin, boundingBox->mMax)) {
                            continue;
                        }
                    }
                }

                loadUniform("M", spatial->getModelMatrix());
                loadUniform("N", spatial->getNormalMatrix());

                loadTexture("alphaMap", renderable->mAlphaMap);
                loadTexture("diffuseMap", renderable->mDiffuseMap);
                loadTexture("specularMap", renderable->mSpecularMap);

                loadUniform("ambientColor", renderable->mMaterial.mAmbient);
                loadUniform("diffuseColor", renderable->mMaterial.mDiffuse);
                loadUniform("specularColor", renderable->mMaterial.mSpecular);
                loadUniform("shininess", renderable->mMaterial.mShininess);

                /* DRAW */
                renderableIt->get<MeshComponent>()->mMesh.draw();
            }

            unbind();
        }
};