#include <Engine.hpp>
#include "Loader/Loader.hpp"

#include "GBufferShader.hpp"
#include "LightPassShader.hpp"
#include "AOShader.hpp"
#include "CombineShader.hpp"
#include "BlurShader.hpp"
#include "DecalShader.hpp"

#include "LightParticleSystem.hpp"
#include "LightParticleShader.hpp"

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
    Engine::addComponent<FrustumComponent>(&camera.camera->getGameObject());

    std::vector<Light *> lights;
    lights.push_back(new Light(glm::vec3(25.f, 25.f, 0.f), glm::vec3(1.f), glm::vec3(100.f)));

    // Sponza object
    {
        auto asset = Loader::loadMultiAsset("sponza.obj");

        // TODO - particle lights rendering
        // TODO - shadows? directional light?
        // TODO - decal bounding box
        // TODO - sized post process
        for (auto& a : asset) {
            GameObject& go = Engine::createGameObject();
            Engine::addComponent<SpatialComponent>(&go, glm::vec3(0.f), glm::vec3(0.2f));
            Engine::addComponent<MeshComponent>(&go, a.mesh);
            Engine::addComponent<GBufferComponent>(&go,
                a.alphaTex ? *a.alphaTex : *Library::getTexture("white"),
                a.diffuseTexture ? *a.diffuseTexture : *Library::getTexture("black"),
                a.specularTexture ? *a.specularTexture : *Library::getTexture("white"),
                a.material
            );
            Engine::addComponent<BoundingBoxComponent>(&go, a.mesh);
        }
    }

    // Decal
    {
        auto gameObject = &Engine::createGameObject();
        Engine::addComponent<SpatialComponent>(gameObject, glm::vec3(0.f), glm::vec3(65.f));
        Engine::addComponent<DecalRenderable>(gameObject, *Library::loadTexture("decal.png"));
        Engine::addComponent<RotationComponent>(gameObject, glm::vec3(0, 1, 0));
    }

    /* Systems - order matters! */
    Engine::addSystem<CameraControllerSystem>();
    Engine::addSystem<FrustumSystem>();
    Engine::addSystem<RotationSystem>();
    Engine::addSystem<LightParticleSystem>();

    /* Init renderer */
    Renderer::init("shaders/");

    // TODO - this ordering is super broken
    Renderer::addPreProcessShader<GBufferShader>("gbuffer.vert", "gbuffer.frag");
    Renderer::addPreProcessShader<DecalShader>("decal.vert", "decal.frag");
    Renderer::addPreProcessShader<LightPassShader>("lightpass.vert", "lightpass.frag");  // run light pass after generating gbuffer
    Renderer::addPreProcessShader<LightParticleShader>("particlebillboard.vert", "particlebillboard.frag");
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
                ImGui::SliderFloat3("Position", glm::value_ptr(pos), -25.f, 25.f);
                ImGui::SliderFloat("Scale", &size, 15.f, 100.f);
                ImGui::SliderFloat3("Color", glm::value_ptr(color), 0.01f, 1.f);
                if (ImGui::Button("Create")) {
                    lights.push_back(new Light(pos, color, glm::vec3(size)));
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
                ImGui::SliderInt("Num lights", &numLights, 0, 1000);
                ImGui::SliderFloat3("Min offset", glm::value_ptr(minOffset), -100.f, 100.f);
                ImGui::SliderFloat3("Max offset", glm::value_ptr(maxOffset), -100.f, 100.f);
                ImGui::SliderFloat("Min scale", &minScale, 0.f, maxScale);
                ImGui::SliderFloat("Max scale", &maxScale, minScale, 100.f);
                if (ImGui::Button("Create light")) {
                    for (int i = 0; i < numLights; i++) {
                        glm::vec3 position = glm::vec3(
                            Util::genRandom(minOffset.x, maxOffset.x),
                            Util::genRandom(minOffset.y, maxOffset.y),
                            Util::genRandom(minOffset.z, maxOffset.z)
                        );
                        lights.push_back(new Light(position, Util::genRandomVec3(), glm::vec3(Util::genRandom(minScale, maxScale))));
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
            spat->setScale(glm::vec3(spat->getScale().x));
        }
    });

    /* Run */
    Engine::run();
    return 0;
}
