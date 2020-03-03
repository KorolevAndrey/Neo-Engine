#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Engine.hpp"

#include "DecalRenderable.hpp"

using namespace neo;

class DecalShader : public Shader {

    public:

        DecalShader(const std::string &vert, const std::string &frag) :
            Shader("DecalShader", vert, frag) {
        }

        virtual void render() override {
            auto fbo = Library::getFBO("gbuffer");
            fbo->bind();
            CHECK_GL(glViewport(0, 0, fbo->mTextures[0]->mWidth, fbo->mTextures[0]->mHeight));
            CHECK_GL(glDepthMask(GL_FALSE));

            bind();

            FrustumComponent* cameraFrustum = nullptr;
            auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>();
            if (camera) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());
                loadUniform("invPV", glm::inverse(camera->get<CameraComponent>()->getProj() * camera->get<CameraComponent>()->getView()));
                cameraFrustum = camera->mGameObject.getComponentByType<FrustumComponent>();
            }

            /* Bind gbuffer */
            auto gbuffer = Library::getFBO("gbuffer");
            loadTexture("gNormal", *gbuffer->mTextures[1]);
            /* On some architectures, binding the depth buffer as a texture will decompress it 
               http://amd-dev.wpengine.netdna-cdn.com/wordpress/media/2013/05/GCNPerformanceTweets.pdf*/
            loadTexture("gDepth",  *gbuffer->mTextures[3]);

            /* Render decals */
            for (auto& decal : Engine::getComponentTuples<DecalRenderable, SpatialComponent>()) {
                auto spatial = decal->get<SpatialComponent>();
                auto boundingBox = decal->mGameObject.getComponentByType<BoundingBoxComponent>();

                // VFC 
                if (cameraFrustum && boundingBox && !cameraFrustum->isInFrustum(spatial->getPosition(), spatial->getScale(), boundingBox->mMin, boundingBox->mMax)) {
                    continue;
                }

                // Flip depth function if camera intersects decal
                glm::vec3 cameraNear = camera->get<SpatialComponent>()->getPosition() + camera->get<SpatialComponent>()->getLookDir() * camera->get<CameraComponent>()->getNearFar().x;
                if (boundingBox && boundingBox->intersect(cameraNear)) {
                    CHECK_GL(glCullFace(GL_FRONT));
                    CHECK_GL(glDepthFunc(GL_ALWAYS));
                }
                else {
                    CHECK_GL(glCullFace(GL_BACK));
                    CHECK_GL(glDepthFunc(GL_LESS));
                }
                loadUniform("M", spatial->getModelMatrix());
                loadUniform("invM", glm::inverse(spatial->getModelMatrix()));

                loadTexture("decalTexture", decal->get<DecalRenderable>()->mDiffuseMap);

                Library::getMesh("cube")->draw();
            }

            unbind();
    }
};
