#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class BCombineShader : public PostProcessShader {

    public:

        BCombineShader(const std::string &frag) :
            PostProcessShader("BCombine Shader", frag) 
        {}

        virtual void render() override {
            loadTexture("linearz", *Library::getFBO("linearZ")->mTextures[0]);
        }

        virtual void imguiEditor() override {
        }
};
