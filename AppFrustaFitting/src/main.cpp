#include <Engine.hpp>
#include "Loader/Loader.hpp"

#include "PerspectiveUpdateSystem.hpp"

#include "Renderer/Shader/ShadowCasterShader.hpp"
#include "Renderer/Shader/PhongShadowShader.hpp"
#include "Renderer/Shader/LineShader.hpp"
#include "Renderer/Shader/WireFrameShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

static constexpr int shadowMapSize = 2048;

/* Game object definitions */
struct Camera {
    GameObject *gameObject;
    CameraComponent *camera;

    Camera(float fov, float near, float far, float ar, glm::vec3 pos) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov, ar);
    }
};

struct Light {
    Light(glm::vec3 position, bool attachCube = true) {
        // Light object
        auto lightObject = &Engine::createGameObject();
        auto& spatial = Engine::addComponent<SpatialComponent>(lightObject, position, glm::vec3(1.f));
        spatial.setLookDir(glm::vec3(0.f, -0.5f, 0.7f));
        Engine::addComponent<LightComponent>(lightObject, glm::vec3(1.f), glm::vec3(0.4f, 0.2f, 0.f));

        // Shadow camera object
        auto cameraObject = &Engine::createGameObject();
        Engine::addComponentAs<OrthoCameraComponent, CameraComponent>(cameraObject, -2.f, 2.f, -4.f, 2.f, 0.1f, 5.f);
        Engine::addComponent<SpatialComponent>(cameraObject, position, glm::vec3(1.f));
        Engine::addComponent<FrustumComponent>(cameraObject);
        Engine::addComponent<FrustumFitReceiverComponent>(cameraObject);
        Engine::addComponent<LineMeshComponent>(cameraObject, glm::vec3(1.f, 0.f, 1.f));
        Engine::addComponent<ShadowCameraComponent>(cameraObject);

        Engine::addImGuiFunc("Light", []() {
            auto light = Engine::getSingleComponent<LightComponent>();
            auto spatial = light->getGameObject().getComponentByType<SpatialComponent>();
            light->imGuiEditor();
            glm::vec3 lookdir = spatial->getLookDir();
            ImGui::SliderFloat3("lookdir", &lookdir[0], -1.f, 1.f);
            spatial->setLookDir(lookdir);
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation = glm::vec3(0.f)) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<MeshComponent>(gameObject, *mesh);
        Engine::addComponent<SpatialComponent>(gameObject, position, scale, rotation);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "ShadowFrustaFitting";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera sceneCamera(45.f, 1.f, 100.f, Window::getAspectRatio(), glm::vec3(0, 0.6f, 5));
    Engine::addComponent<CameraControllerComponent>(sceneCamera.gameObject, 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(sceneCamera.gameObject);
    
    // Perspective camera
    Camera mockCamera(50.f, 0.1f, 5.f, 1.f, glm::vec3(0.f, 2.f, -0.f));
    auto* line = &Engine::addComponent<LineMeshComponent>(mockCamera.gameObject, glm::vec3(0.f, 1.f, 1.f));
    Engine::addComponent<FrustumComponent>(mockCamera.gameObject);
    Engine::addComponent<FrustumFitSourceComponent>(mockCamera.gameObject);

    // Ortho camera, shadow camera, light
    Light light(glm::vec3(10.f, 20.f, 0.f), true);

    /* Sponza object */
    {
        auto asset = Loader::loadMultiAsset("sponza.obj");

        // TODO - these should have some parent/child thing going on..
        for (auto& a : asset) {
            GameObject& go = Engine::createGameObject();
            Engine::addComponent<SpatialComponent>(&go, glm::vec3(0.f), glm::vec3(0.2f));
            Engine::addComponent<MeshComponent>(&go, a.mesh);
            Engine::addComponent<renderable::PhongShadowRenderable>(&go, a.diffuseTexture ? *a.diffuseTexture : *Library::getTexture("black"), a.material);
            Engine::addComponent<renderable::ShadowCasterRenderable>(&go, a.alphaTex ? *a.alphaTex : *Library::getTexture("white"));
        }
    }

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>(); // Update camera
    Engine::addSystem<FrustumSystem>(); // Calculate original frusta bounds
    Engine::addSystem<FrustaFittingSystem>(); // Fit one frusta into another
    Engine::addSystem<FrustumToLineSystem>(); // Create line mesh
    auto& perspectiveUpdate = Engine::addSystem<PerspectiveUpdateSystem>(); // Update mock perspective camera

    /* Init renderer */
    Renderer::init("shaders/");
    Renderer::addPreProcessShader<ShadowCasterShader>(shadowMapSize);
    Renderer::addSceneShader<PhongShadowShader>();
    Renderer::addSceneShader<LineShader>();

    /* Attach ImGui panes */
    Engine::addImGuiFunc("SceneCamera", [&]() {
        if (ImGui::Button("Set scene")) {
            Engine::removeComponent<MainCameraComponent>(*mockCamera.gameObject->getComponentByType<MainCameraComponent>());
            Engine::removeComponent<CameraControllerComponent>(*mockCamera.gameObject->getComponentByType<CameraControllerComponent>());
            Engine::removeComponent<FrustumFitSourceComponent>(*mockCamera.gameObject->getComponentByType<FrustumFitSourceComponent>());
            if (!sceneCamera.gameObject->getComponentByType<MainCameraComponent>()) {
                Engine::addComponent<MainCameraComponent>(sceneCamera.gameObject);
            }
            if (!sceneCamera.gameObject->getComponentByType<CameraControllerComponent>()) {
                Engine::addComponent<CameraControllerComponent>(sceneCamera.gameObject, 0.4f, 7.f);
            }
            if (!sceneCamera.gameObject->getComponentByType<FrustumFitSourceComponent>()) {
                Engine::addComponent<FrustumFitSourceComponent>(sceneCamera.gameObject);
            }
        }
        if (ImGui::Button("Set perspective")) {
            Engine::removeComponent<MainCameraComponent>(*sceneCamera.gameObject->getComponentByType<MainCameraComponent>());
            Engine::removeComponent<CameraControllerComponent>(*sceneCamera.gameObject->getComponentByType<CameraControllerComponent>());
            Engine::removeComponent<FrustumFitSourceComponent>(*sceneCamera.gameObject->getComponentByType<FrustumFitSourceComponent>());
            if (!mockCamera.gameObject->getComponentByType<MainCameraComponent>()) {
                Engine::addComponent<MainCameraComponent>(mockCamera.gameObject);
            }
            if (!mockCamera.gameObject->getComponentByType<CameraControllerComponent>()) {
                Engine::addComponent<CameraControllerComponent>(mockCamera.gameObject, 0.4f, 7.f);
            }
            if (!mockCamera.gameObject->getComponentByType<FrustumFitSourceComponent>()) {
                Engine::addComponent<FrustumFitSourceComponent>(mockCamera.gameObject);
            }
        }

    });
    Engine::addImGuiFunc("PerspectiveCamera", [&]() {
        auto spatial = mockCamera.gameObject->getComponentByType<SpatialComponent>();
        auto camera = dynamic_cast<PerspectiveCameraComponent*>(mockCamera.camera);
        ImGui::Checkbox("Auto update", &perspectiveUpdate.mUpdatePerspective);
        spatial->imGuiEditor();
        camera->imGuiEditor();
    });

    /* Run */
    Engine::run();
    return 0;
}
