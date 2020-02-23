#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

    class CameraControllerSystem : public System {

    public:
        CameraControllerSystem() :
            System("CameraController System")
        {}

        virtual void update(const float dt) override;
        virtual void imguiEditor() override;

        float mSuperSpeed = 2.5f;

    protected:
        void _updateLook(const float dt, CameraControllerComponent&);
        void _updatePosition(const float dt, CameraControllerComponent&);


    };
}