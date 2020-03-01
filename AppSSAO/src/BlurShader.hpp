#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

using namespace neo;

class BlurShader : public PostProcessShader {

    public:

        int blurAmount = 4;

        // TODO - this is full-res..
        BlurShader(const std::string &frag) :
            PostProcessShader("Blur Shader", frag) 
        {}

        virtual void render() override {
            loadUniform("blurAmount", blurAmount);
        }

        virtual void imguiEditor() override {
            ImGui::SliderInt("Blur", &blurAmount, 0, 10);
        }
};