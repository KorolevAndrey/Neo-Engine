#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class BLinearZShader : public PostProcessShader {

    public:

        BLinearZShader(const std::string &frag) :
            PostProcessShader("BLinearZ Shader", frag) 
        {}

        virtual void render() override {
            if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("zNear", camera->get<CameraComponent>()->getNearFar().x);
                loadUniform("zFar", camera->get<CameraComponent>()->getNearFar().y);
            }
        }

        virtual void imguiEditor() override {
        }
};
