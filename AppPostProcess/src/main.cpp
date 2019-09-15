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
        camera = &Engine::addComponentAs<PerspectiveCameraComponent, CameraComponent>(gameObject, near, far, fov);
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
            glm::vec3 pos = gameObject->getComponentByType<SpatialComponent>()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f)) {
                gameObject->getComponentByType<SpatialComponent>()->setPosition(pos);
            }
            ImGui::SliderFloat3("Color", glm::value_ptr(light->mColor), 0.f, 1.f);
            ImGui::SliderFloat3("Attenuation", glm::value_ptr(light->mAttenuation), 0.f, 1.f);
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

        Engine::addImGuiFunc("Mesh", [&]() {
            glm::vec3 pos = gameObject->getComponentByType<SpatialComponent>()->getPosition();
            if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.f, 10.f)) {
                gameObject->getComponentByType<SpatialComponent>()->setPosition(pos);
            }
            float scale = gameObject->getComponentByType<SpatialComponent>()->getScale().x;
            if (ImGui::SliderFloat("Scale", &scale, 0.f, 10.f)) {
                gameObject->getComponentByType<SpatialComponent>()->setScale(glm::vec3(scale));
            }
            static glm::vec3 rot(0.f);
            if (ImGui::SliderFloat3("Rotation", glm::value_ptr(rot), 0.f, 4.f)) {
                glm::mat4 R;
                R = glm::rotate(glm::mat4(1.f), rot.x, glm::vec3(1, 0, 0));
                R *= glm::rotate(glm::mat4(1.f), rot.y, glm::vec3(0, 1, 0));
                R *= glm::rotate(glm::mat4(1.f), rot.z, glm::vec3(0, 0, 1));
                gameObject->getComponentByType<SpatialComponent>()->setOrientation(glm::mat3(R));
            }
        });
    }
};

int main() {
    Engine::init("Post Process", "res/", 1280, 720);

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
    Engine::addDefaultImGuiFunc();

    /* Run */
    Engine::run();
    return 0;
}