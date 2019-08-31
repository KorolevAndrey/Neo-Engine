#include <Engine.hpp>
#include "CameraControllerSystem.hpp"

namespace neo {

    void CameraControllerSystem::update(const float dt) {
        glm::vec2 mousePos = Mouse::getPos();
        glm::vec2 mouseSpeed = Mouse::getSpeed();
 
        for (auto comp : Engine::getComponents<CameraControllerComponent>()) {
            if (Mouse::isDown(GLFW_MOUSE_BUTTON_1) && (mousePos.x || mousePos.y)) {
                float theta = comp->mTheta - mouseSpeed.x * comp->mLookSpeed * dt;
                float phi = comp->mPhi + mouseSpeed.y * comp->mLookSpeed * dt;
                comp->setOrientation(theta, phi);
            }

            int forward(Keyboard::isKeyPressed(comp->mForwardButton));
            int backward(Keyboard::isKeyPressed(comp->mBackwardButton));
            int right(Keyboard::isKeyPressed(comp->mRightButton));
            int left(Keyboard::isKeyPressed(comp->mLeftButton));
            int up(Keyboard::isKeyPressed(comp->mUpButton));
            int down(Keyboard::isKeyPressed(comp->mDownButton));
            int speed(Keyboard::isKeyPressed(GLFW_KEY_LEFT_SHIFT));

            glm::vec3 dir(
                float(right - left),
                float(up - down),
                float(backward - forward)
            );

            if (dir != glm::vec3()) {
                auto spatial = comp->getGameObject().getSpatial();
                dir = glm::normalize(dir);
                dir = glm::normalize(
                    spatial->getRightDir() * dir.x +
                    spatial->getUpDir() * dir.y +
                    -spatial->getLookDir() * dir.z);
                comp->getGameObject().getSpatial()->move(dir * comp->mMoveSpeed * dt * (speed ? mSuperSpeed : 1.f));
            }
        }
    }

    void CameraControllerSystem::imguiEditor() {
        ImGui::SliderFloat("SuperSpeed", &mSuperSpeed, 1.f, 10.f);
    }

}
