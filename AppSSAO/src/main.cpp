#include <Engine.hpp>

#include "Renderer/Shader/WireframeShader.hpp"
#include "GBufferShader.hpp"
#include "LightPassShader.hpp"
#include "AOShader.hpp"
#include "CombineShader.hpp"
#include "BlurShader.hpp"
#include "GBufferComponent.hpp"

#include "Loader/Loader.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "Util/Util.hpp"

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

    Light(glm::vec3 pos, glm::vec3 col, glm::vec3 scale) {
        gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, pos, scale);
        light = &Engine::addComponent<LightComponent>(gameObject, col);
    }
};

int main() {
    EngineConfig config;
    config.APP_NAME = "SSAO";
    config.APP_RES = "res/";
    Engine::init(config);

    /* Game objects */
    Camera camera(45.f, 1.f, 1000.f, glm::vec3(0, 0.6f, 5), 0.4f, 20.f);
    Engine::addComponent<MainCameraComponent>(&camera.camera->getGameObject());

    std::vector<Light *> lights;
    lights.push_back(new Light(glm::vec3(25.f, 25.f, 0.f), glm::vec3(1.f), glm::vec3(100.f)));
    /* Sponza object */
    {
        auto asset = Loader::loadMultiAsset("sponza.obj");

        GameObject& parent = Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(&parent, glm::vec3(0.f), glm::vec3(0.2f));

        for (auto& a : asset) {
            auto& renderable = Engine::addComponent<GBufferComponent>(&parent);
            renderable.mesh = a.mesh;
            renderable.material = a.material;
            if (a.diffuseTexture) {
                renderable.diffuseMap = a.diffuseTexture;
            }
            if (a.displacementTexture) {
                renderable.normalMap = a.displacementTexture;
            }
        }
    }

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<SinTranslateSystem>();

    /* Init renderer */
    Renderer::init("shaders/");

    // TODO - this ordering is super broken
    Renderer::addPreProcessShader<GBufferShader>("gbuffer.vert", "gbuffer.frag");
    Renderer::addPreProcessShader<LightPassShader>("lightpass.vert", "lightpass.frag");  // run light pass after generating gbuffer
    Renderer::addPostProcessShader<AOShader>("ao.frag");    // first post process - generate ssao map 
    Renderer::addPostProcessShader<BlurShader>("blur.frag"); // blur ssao map
    Renderer::addPostProcessShader<CombineShader>("combine.frag");    // combine light pass and ssao 

    /* Attach ImGui panes */
    Engine::addImGuiFunc("Lights", [&]() {
        static int index;
        if (ImGui::CollapsingHeader("Create Lights")) {
            if (ImGui::TreeNode("Single")) {
                static glm::vec3 pos(0.f);
                static float size(15.f);
                static glm::vec3 color(1.f);
                static float yOffset(10.f);
                ImGui::SliderFloat3("Position", glm::value_ptr(pos), -25.f, 25.f);
                ImGui::SliderFloat("Scale", &size, 15.f, 100.f);
                ImGui::SliderFloat3("Color", glm::value_ptr(color), 0.01f, 1.f);
                ImGui::SliderFloat("Offset", &yOffset, 0.f, 25.f);
                if (ImGui::Button("Create")) {
                    auto light = new Light(pos, color, glm::vec3(size));
                    if (yOffset) {
                        Engine::addComponent<SinTranslateComponent>(light->gameObject, glm::vec3(0.f, yOffset, 0.f), pos);
                    }
                    lights.push_back(light);
                    index = lights.size() - 1;
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Random Lights")) {
                if (ImGui::Button("Clear lights")) {
                    for (auto & l : lights) {
                        Engine::removeGameObject(*l->gameObject);
                    }
                    lights.clear();
                }
                static int numLights = 10;
                static glm::vec3 minOffset(0.f);
                static glm::vec3 maxOffset(0.f);
                static float minScale(0.f);
                static float maxScale(10.f);
                static float minSinOffset(0.f);
                static float maxSinOffset(0.f);
                ImGui::SliderInt("Num lights", &numLights, 0, 1000);
                ImGui::SliderFloat3("Min offset", glm::value_ptr(minOffset), -100.f, 100.f);
                ImGui::SliderFloat3("Max offset", glm::value_ptr(maxOffset), -100.f, 100.f);
                ImGui::SliderFloat("Min scale", &minScale, 0.f, maxScale);
                ImGui::SliderFloat("Max scale", &maxScale, minScale, 100.f);
                ImGui::SliderFloat("Min sin", &minSinOffset, 0.f, 15.f);
                ImGui::SliderFloat("Max sin", &maxSinOffset, 0.f, 15.f);
                if (ImGui::Button("Create light")) {
                    for (int i = 0; i < numLights; i++) {
                        glm::vec3 position = glm::vec3(
                            Util::genRandom(minOffset.x, maxOffset.x),
                            Util::genRandom(minOffset.y, maxOffset.y),
                            Util::genRandom(minOffset.z, maxOffset.z)
                        );
                        glm::vec3 color = Util::genRandomVec3();
                        float size = Util::genRandom(minScale, maxScale);
                        auto light = new Light(position, color, glm::vec3(size));
                        Engine::addComponent<SinTranslateComponent>(light->gameObject, glm::vec3(0.f, Util::genRandom(minSinOffset, maxSinOffset), 0.f), position);
                        lights.push_back(light);
                    }
                }
                ImGui::TreePop();
            }
        }
        if (lights.empty()) {
            return;
        }
        if (ImGui::CollapsingHeader("Edit Lights")) {
            ImGui::SliderInt("Index", &index, 0, lights.size() - 1);
            auto l = lights[index];
            if (ImGui::Button("Delete light")) {
                Engine::removeGameObject(*l->gameObject);
                lights.erase(lights.begin() + index);
                index = glm::max(0, index - 1);
            }
            l->light->imGuiEditor();
            auto spat = l->gameObject->getComponentByType<SpatialComponent>();
            if (!spat) {
                return;
            }
            spat->imGuiEditor();
        }
    });

    /* Run */
    Engine::run();
    return 0;
}
