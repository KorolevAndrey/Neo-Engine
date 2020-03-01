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

            CHECK_GL(glDisable(GL_CULL_FACE));
            CHECK_GL(glDepthMask(GL_FALSE));
            CHECK_GL(glDepthFunc(GL_LEQUAL));

            bind();

            if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("invPV", glm::inverse(camera->get<CameraComponent>()->getProj() * camera->get<CameraComponent>()->getView()));
                loadUniform("V", camera->get<CameraComponent>()->getView());
            }

            /* Bind gbuffer */
            auto gbuffer = Library::getFBO("gbuffer");
            loadTexture("gNormal", *gbuffer->mTextures[1]);
            loadTexture("gDepth",  *gbuffer->mTextures[3]);

            /* Render decals */
            for (auto& decal : Engine::getComponentTuples<DecalRenderable, SpatialComponent>()) {
                auto spatial = decal->get<SpatialComponent>();
                loadUniform("M", spatial->getModelMatrix());
                loadUniform("invM", glm::inverse(spatial->getModelMatrix()));

                loadTexture("decalTexture", decal->get<DecalRenderable>()->mDiffuseMap);

                Library::getMesh("cube")->draw();
            }

            unbind();
    }
};
