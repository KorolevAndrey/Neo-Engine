#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class CombineShader : public PostProcessShader {

    public:

        bool showAO = true;

        CombineShader(const std::string &frag) :
            PostProcessShader ("Combine Shader", frag) 
        {}

        virtual void render() override {
            loadUniform("showAO", showAO);

            // Bind diffuse output
            loadTexture("gDiffuse", *Library::getFBO("gbuffer")->mTextures[0]);

            // Bind light pass output
            loadTexture("lightOutput", *Library::getFBO("lightpass")->mTextures[0]);
        }

        virtual void imguiEditor() override {
            ImGui::Checkbox("Show AO", &showAO);
        }
};
