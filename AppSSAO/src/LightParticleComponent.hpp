#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

struct BaseParticle {
    float scale;
    glm::vec3 color;
    glm::vec3 startPosition;
    glm::vec3 endPosition;
    float positionJitter;
};

class LightParticleComponent: public Component {
public:
    float lifeSpan = 1.f;
    float life = 0.f;
    const BaseParticle base;

    LightParticleComponent(GameObject *go, float lifeSpan, const BaseParticle base) :
        Component(go),
        lifeSpan(lifeSpan),
        base(base)
    {}
};