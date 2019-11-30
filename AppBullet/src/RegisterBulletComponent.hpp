#pragma once

#include "GameObject/GameObject.hpp"
#include "Component/Component.hpp"

using namespace neo;

class RegisterBulletComponent : public Component {
public:

    RegisterBulletComponent(GameObject* go) :
        Component(go)
    {
    }
};
