#include <Engine.hpp>

#include "Shader/PhongShader.hpp"
#include "Shader/PostProcessShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

/* Game object definitions */
struct Camera {
    CameraComponent *camera;
    Camera(float fov, float near, float far, glm::vec3 pos, float ls, float ms) {
        GameObject *gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, glm::vec3(1.f));
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov, Window::getAspectRatio());
        Engine::addComponent<CameraControllerComponent>(gameObject, ls, ms);
    }
};

struct Light {
    GameObject *gameObject;
    LightComponent *light;

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos);
        light = &Engine::addComponent<LightComponent>(gameObject, col, att);

        Engine::addImGuiFunc("Light", [&]() {
            if (!light) {
                return;
            }
            light->imGuiEditor();
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh, float amb, glm::vec3 diffuse, glm::vec3 spec) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(1.f));
        Engine::addComponent<MeshComponent>(gameObject, mesh);
        Engine::addComponent<renderable::PhongRenderable>(gameObject);
        Engine::addComponent<MaterialComponent>(gameObject, amb, diffuse, spec);
    }
};

int main() {
	EngineConfig config;
	config.APP_NAME = "Post Process";
	config.APP_RES = "res/";
	Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));
    Renderable r(Library::getMesh("mr_krab.obj"), 0.2f, glm::vec3(1.f, 0.f, 1.f), glm::vec3(1.f));
    Engine::addComponent<DiffuseMapComponent>(r.gameObject, Library::getTexture("mr_krab.png"));

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();

    /* Init renderer */
    Renderer::init("shaders/", camera.camera);
    Renderer::addSceneShader<PhongShader>();
    Renderer::addPostProcessShader<PostProcessShader>("DepthShader", "depth.frag");
    Renderer::addPostProcessShader<PostProcessShader>("BlueShader", "blue.frag");
    Renderer::addPostProcessShader<PostProcessShader>("InvertShader", "invert.frag");

    /* Attach ImGui panes */

    /* Run */
    Engine::run();
    return 0;
}