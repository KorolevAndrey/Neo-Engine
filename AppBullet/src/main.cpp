#include <Engine.hpp>

#include "BulletSystem.hpp"
#include "BulletShader.hpp"
#include "Shader/PhongShader.hpp"
#include "Shader/AlphaTestShader.hpp"

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
    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 att) {
        auto& gameObject = Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(&gameObject, pos);
        Engine::addComponent<LightComponent>(&gameObject, col, att);

        Engine::addImGuiFunc("Light", [&]() {
            auto light = Engine::getSingleComponent<LightComponent>();
            light->imGuiEditor();
            if (auto spatial = light->getGameObject().getComponentByType<SpatialComponent>()) {
                spatial->imGuiEditor();
            }
        });
    }
};

struct Renderable {
    GameObject *gameObject;

    Renderable(Mesh *mesh, glm::vec3 position = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), glm::vec3 rotation = glm::vec3(0.f)) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<MeshComponent>(gameObject, mesh);
        Engine::addComponent<SpatialComponent>(gameObject, position, scale, rotation);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "Bullet";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 100.f, glm::vec3(0, 0.6f, 5), 0.4f, 7.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    Light(glm::vec3(0.f, 2.f, 20.f), glm::vec3(1.f), glm::vec3(0.6, 0.2, 0.f));

    /* Cube object */
    for (int i = 0; i < 10; i++) {
        Renderable cube(Library::getMesh("cube"), glm::vec3(Util::genRandom(-1.f, 1.f), i * 2, Util::genRandom(-1.f, 1.f)));
        Engine::addComponent<renderable::PhongRenderable>(cube.gameObject);
        Engine::addComponent<MaterialComponent>(cube.gameObject, 0.2f, Util::genRandomVec3(), glm::vec3(1.f));
        Engine::addComponent<BulletCubeRigidBodyComponent>(cube.gameObject);
        Engine::addComponent<RegisterBulletComponent>(cube.gameObject);
    }

    /* Ground plane */
    Renderable plane(Library::getMesh("quad"), glm::vec3(0.f), glm::vec3(15.f), glm::vec3(-Util::PI() / 2.f, 0.f, 0.f));
    Engine::addComponent<renderable::AlphaTestRenderable>(plane.gameObject);
    Engine::addComponent<DiffuseMapComponent>(plane.gameObject, *Library::getTexture("grid.png"));
    Engine::addComponent<BulletCubeRigidBodyComponent>(plane.gameObject, 0.f, glm::vec3(0.f));
    Engine::addComponent<RegisterBulletComponent>(plane.gameObject);

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<BulletSystem>();

    /* Init renderer */
    Renderer::init("shaders/", camera.camera);
    Renderer::addSceneShader<PhongShader>();
    Renderer::addSceneShader<AlphaTestShader>();
    Renderer::addSceneShader<BulletShader>();

    /* Run */
    Engine::run();
    return 0;
}