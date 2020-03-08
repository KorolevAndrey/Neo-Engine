#pragma once

#include "ECS/Systems/System.hpp"
#include "Engine.hpp"

#include "LightParticleComponent.hpp"

using namespace neo;

class LightParticleSystem : public System {

public:
    const std::vector<BaseParticle> baseParticles = {
        { 60.f, glm::vec3(0.9f, 0.56f, 0.25f), glm::vec3(-240.44, 30.f,  80.76), glm::vec3(-240.44, 60.f,  80.76), 10.f },
        { 60.f, glm::vec3(0.9f, 0.56f, 0.25f), glm::vec3(-240.44, 30.f, -89.80), glm::vec3(-240.44, 60.f, -89.80), 10.f },
        { 60.f, glm::vec3(0.9f, 0.56f, 0.25f), glm::vec3( 223.94, 30.f, -89.80), glm::vec3( 223.94, 60.f, -89.80), 10.f },
        { 60.f, glm::vec3(0.9f, 0.56f, 0.25f), glm::vec3( 223.94, 30.f,  80.76), glm::vec3( 223.94, 60.f,  80.76), 10.f },
        { 20.f, glm::vec3(0.8f, 0.24f, 0.18f), glm::vec3(  97.1f, 25.f,  28.7f), glm::vec3(  97.1f, 50.f,  28.7f), 1.5f },
        { 20.f, glm::vec3(0.8f, 0.24f, 0.18f), glm::vec3(  97.1f, 25.f, -43.9f), glm::vec3(  97.1f, 50.f, -43.9f), 1.5f },
        { 20.f, glm::vec3(0.8f, 0.24f, 0.18f), glm::vec3(-123.4f, 25.f, -43.9f), glm::vec3(-123.4f, 50.f, -43.9f), 1.5f },
        { 20.f, glm::vec3(0.8f, 0.24f, 0.18f), glm::vec3(-123.4f, 25.f,  28.6f), glm::vec3(-123.4f, 50.f,  28.6f), 1.5f }
    };

    float particleMinLife = 5.f;
    float particleMaxLife = 10.f;
    float colorJitter = 0.4f;
    int maxLights = 500;

    LightParticleSystem() :
        System("LightParticle System") {
    }

    virtual void init() override {
        for (int i = 0; i < static_cast<int>(Util::genRandom(maxLights / 4, maxLights)); i++) {
            _createLight();
        }
    }

    virtual void update(const float dt) override {
        auto lights = Engine::getComponentTuples<LightParticleComponent, LightComponent, SpatialComponent>();

        for (auto&& light : lights) {
            auto&& lightParticle = light->get<LightParticleComponent>();
            if (lightParticle->life > lightParticle->lifeSpan) {
                Engine::removeGameObject(light->mGameObject);
                continue;
            }
 
            auto&& spatial = light->get<SpatialComponent>();

            lightParticle->life += Util::getRunTime() / 1000.f;

            spatial->setPosition(lightParticle->base.startPosition + glm::normalize(lightParticle->base.endPosition - lightParticle->base.startPosition) * glm::distance(lightParticle->base.endPosition, lightParticle->base.startPosition) * lightParticle->life / lightParticle->lifeSpan);
            spatial->setScale(lightParticle->base.scale * (1.0 - lightParticle->life / lightParticle->lifeSpan));

            light->get<LightComponent>()->mColor *= 0.99f;
       }

        for (int i = lights.size(); i < maxLights; i++) {
            _createLight();
        }
    }

    void _createLight() {
        auto go = &Engine::createGameObject();
        auto randomBase = baseParticles[static_cast<int>(Util::genRandom(0.f, static_cast<float>(baseParticles.size())))];
        randomBase.startPosition += glm::vec3(Util::genRandom(-randomBase.positionJitter, randomBase.positionJitter), 0.f, Util::genRandom(-randomBase.positionJitter, randomBase.positionJitter));
        randomBase.endPosition   += glm::vec3(Util::genRandom(-randomBase.positionJitter, randomBase.positionJitter) / 2.f, 0.f, Util::genRandom(-randomBase.positionJitter, randomBase.positionJitter) / 2.f);
        randomBase.scale = Util::genRandom(randomBase.scale / 3.f, randomBase.scale);
        randomBase.color.y += Util::genRandom(-colorJitter, colorJitter);
        
        Engine::addComponent<SpatialComponent>(go, randomBase.startPosition, glm::vec3(randomBase.scale));
        Engine::addComponent<LightParticleComponent>(go, Util::genRandom(particleMinLife, particleMaxLife), randomBase);
        Engine::addComponent<LightComponent>(go, randomBase.color);
    }

    virtual void imguiEditor() override {
        ImGui::SliderFloat("Particle min life", &particleMinLife, 1.f, 5.f);
        ImGui::SliderFloat("Particle max life", &particleMaxLife, particleMinLife, 10.f);
        ImGui::SliderInt("Number lights", &maxLights, 50, 1000);
        ImGui::SliderFloat("Color jitter", &colorJitter, 0.f, 1.f);
    }
};
