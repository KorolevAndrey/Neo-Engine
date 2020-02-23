#include <Engine.hpp>
#include "Loader/Loader.hpp"

#include "Renderer/Shader/GammaCorrectShader.hpp"

#include "SunComponent.hpp"
#include "SunOccluderComponent.hpp"
#include "nvidia/AGodRaySunShader.hpp"
#include "nvidia/AGodRayOccluderShader.hpp"
#include "nvidia/ABlurShader.hpp"
#include "nvidia/ACombineShader.hpp"

#include "MyPhongRenderable.hpp"
#include "MyPhongShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    CameraComponent *camera;
    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        GameObject *gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov, Window::getAspectRatio());
        auto& cameraController = Engine::addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    Light(glm::vec3 pos, float scale, glm::vec3 col, glm::vec3 att) {
        auto& gameObject = Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(&gameObject, pos, glm::vec3(scale));
        Engine::addComponent<LightComponent>(&gameObject, col, att);
        Engine::addComponent<SunComponent>(&gameObject);

        Engine::addImGuiFunc("Light", [&]() {
            auto light = Engine::getSingleComponent<LightComponent>();
            if (auto spatial = light->getGameObject().getComponentByType<SpatialComponent>()) {
                glm::vec3 pos = spatial->getPosition();
                if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f)) {
                    spatial->setPosition(pos);
                }
                float scale = spatial->getScale().x;
                if (ImGui::SliderFloat("Scale", &scale, 0.f, 100.f)) {
                    spatial->setScale(glm::vec3(scale));
                }
                ImGui::SliderFloat3("Color", glm::value_ptr(light->mColor), 0.f, 1.f);
                ImGui::SliderFloat3("Attenuation", glm::value_ptr(light->mAttenuation), 0.f, 1.f);
            }
        });
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "GodRays";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 500.f, glm::vec3(0, 0.6f, 5), 0.4f, 0.7f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(glm::vec3(150.f, 300.f, -50.f), 45.f, glm::vec3(1.f), glm::vec3(0.f));

    /* Sponza object */
    {
        auto asset = Loader::loadMultiAsset("sponza.obj");

        // TODO - these should have some parent/child thing going on..
        for (auto& a : asset) {
            if (a.diffuseTexture && a.ambientTexture && a.displacementTexture) {
                GameObject& go = Engine::createGameObject();
                Engine::addComponent<SpatialComponent>(&go, glm::vec3(0.f), glm::vec3(0.2f));
                Engine::addComponent<MeshComponent>(&go, *a.mesh);
                Engine::addComponent<MyPhongRenderable>(&go, *a.alphaTex, *a.ambientTexture, *a.diffuseTexture, *a.displacementTexture, *a.specularTexture, a.material);
                Engine::addComponent<SunOccluderComponent>(&go, *a.diffuseTexture);
            }
        }
    }

    /* Systems - order matters! */
    auto& camSystem = Engine::addSystem<CameraControllerSystem>();
    camSystem.mSuperSpeed = 15.f;

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addPreProcessShader<AGodRaySunShader>("billboard.vert", "nvidia/godraysun.frag");
    Renderer::addPreProcessShader<AGodRayOccluderShader>("model.vert", "nvidia/godrayoccluder.frag");
    Renderer::addPreProcessShader<ABlurShader>("nvidia/blur.vert", "nvidia/blur.frag");
    Renderer::addSceneShader<MyPhongShader>("myphong.vert", "myphong.frag");
    Renderer::addPostProcessShader<ACombineShader>("nvidia/combine.frag");
    auto& gamma = Renderer::addPostProcessShader<GammaCorrectShader>();
    gamma.gamma = 1.5f;

    /* Attach ImGui panes */
    Engine::addImGuiFunc("Godray", []() {
        static bool useNvidia = true;
        if (ImGui::RadioButton("Use nvidia", useNvidia)) {
            useNvidia = true;
            Renderer::getShader<AGodRaySunShader>().mActive = true;
            Renderer::getShader<AGodRayOccluderShader>().mActive = true;
            Renderer::getShader<ABlurShader>().mActive = true;
            Renderer::getShader<ACombineShader>().mActive = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Use intel", !useNvidia)) {
            useNvidia = false;
            Renderer::getShader<AGodRaySunShader>().mActive = false;
            Renderer::getShader<AGodRayOccluderShader>().mActive = false;
            Renderer::getShader<ABlurShader>().mActive = false;
            Renderer::getShader<ACombineShader>().mActive = false;
        }
    });

    /* Run */
    Engine::run();
    return 0;
}
