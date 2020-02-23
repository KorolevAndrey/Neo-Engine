#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class BLinearZShader : public Shader {

    public:

        BLinearZShader(const std::string &frag) :
            Shader("BLinearZ Shader")
        {
            _attachStage(ShaderStage::VERTEX, R"(
                layout (location = 0) in vec3 vertPos;
                layout (location = 2) in vec2 vertTex;
                out vec2 fragTex;
                void main() { gl_Position = vec4(2 * vertPos, 1); fragTex = vertTex; })");
            _attachStage(ShaderStage::FRAGMENT, frag);
            init();

            auto fbo = Library::createFBO("linearZ");
            TextureFormat format;
            format.mSizedFormat = GL_R32F;
            format.mBaseFormat = GL_RED;
            fbo->attachColorTexture(Window::getFrameSize(), format);
            fbo->initDrawBuffers();
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("linearZ")->resize(frameSize);
            });
        }

        virtual void render() override {
            bind(); 

            Library::getFBO("linearZ")->bind();
            glm::ivec2 frameSize = Window::getFrameSize();
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            loadTexture("inputFBO", *Library::getFBO("default")->mTextures[0]);
            loadTexture("inputDepth", *Library::getFBO("default")->mTextures[1]);

            if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("zNear", camera->get<CameraComponent>()->getNearFar().x);
                loadUniform("zFar", camera->get<CameraComponent>()->getNearFar().y);
            }

            Library::getMesh("quad")->draw();
        }

        virtual void imguiEditor() override {
        }
};
