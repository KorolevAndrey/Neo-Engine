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
            // Bind light pass output
            loadTexture("lightOutput", showAO ? *Library::getFBO("lightpass")->mTextures[0] : *Library::getTexture("white"));
        }

        virtual void imguiEditor() override {
            ImGui::Checkbox("Show AO", &showAO);
        }
};
