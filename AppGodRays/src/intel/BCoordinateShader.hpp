#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class BCoordinateShader : public Shader {

    public:

        int numSamples = 5;
        int numSlices = 12;

        BCoordinateShader(const std::string &frag) :
            Shader("BCoordinate Shader")
        {
            _attachStage(ShaderStage::VERTEX, R"(
                layout (location = 0) in vec3 vertPos;
                layout (location = 2) in vec2 vertTex;
                out vec2 fragTex;
                void main() { gl_Position = vec4(2 * vertPos, 1); fragTex = vertTex; })");
            _attachStage(ShaderStage::FRAGMENT, frag);
            init();

            auto fbo = Library::createFBO("coordinate");
            TextureFormat format = { GL_RGBA32F, GL_RGBA, GL_NEAREST, GL_REPEAT };
            fbo->attachColorTexture(Window::getFrameSize(), format);

            // Starting with epipolar coordinates first

            // {
            //     TextureFormat format;
            //     format.mSizedFormat = GL_RG32F;
            //     format.mBaseFormat = GL_RG;
            //     fbo->attachColorTexture(glm::uvec2(numSamples, numSlices), format);
            // }
            // {
            //     fbo->attachStencilTexture(glm::uvec2(numSamples, numSlices), GL_NEAREST, GL_REPEAT);
            // }
            fbo->initDrawBuffers();
        }

        virtual void render() override {
            bind(); 

            Library::getFBO("coordinate")->bind();
            glm::ivec2 frameSize = Window::getFrameSize();
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            loadUniform("numSamples", numSamples);
            loadUniform("numSlices", numSlices);
            loadUniform("screenSize", glm::vec2(frameSize));
            loadTexture("linearz", *Library::getFBO("linearZ")->mTextures[0]);

            if (auto light = Engine::getComponentTuple<LightComponent, SpatialComponent>()) {
                loadUniform("lightPos", light->get<SpatialComponent>()->getPosition());
            }
            if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("V", camera->get<CameraComponent>()->getView());
            }

            Library::getMesh("quad")->draw();
        }

        virtual void imguiEditor() override {
            ImGui::SliderInt("Samples", &numSamples, 1, 64);
            ImGui::SliderInt("Slices", &numSlices, 1, 64);
        }
};
