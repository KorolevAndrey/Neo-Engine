#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class ACombineShader : public PostProcessShader {

    public:

        float exposure = 0.9f;

        ACombineShader(const std::string &frag) :
            PostProcessShader("ACombine Shader", frag) 
        {}

        virtual void render() override {
            loadTexture("godray", *Library::getFBO("ablur")->mTextures[0]);

            loadUniform("sunColor", Engine::getSingleComponent<SunComponent>()->getGameObject().getComponentByType<LightComponent>()->mColor);
            loadUniform("exposure", exposure);
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Exposure", &exposure, 0.01f, 10.f);
        }
};
