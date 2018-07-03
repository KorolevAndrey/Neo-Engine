#include "ThirdPersonCamera.hpp"

ThirdPersonCamera::ThirdPersonCamera(glm::vec3 *ref) {
    this->lookRef = ref;
    update();
}

ThirdPersonCamera::ThirdPersonCamera(glm::vec3 *ref, const glm::vec3 pos) :
    ThirdPersonCamera(ref) {
    this->position = pos;
}

void ThirdPersonCamera::update() {
    Camera::updateUVW();
    updateLookAt();
    distanceFromBase = glm::distance(position, lookAt);
    
    float thetaR = (float) glm::radians(theta);
    float phiR = (float) glm::radians(phi);
    float horz = distanceFromBase * glm::cos(phiR);
    float vert = distanceFromBase * glm::sin(phiR);
    
    position.x = lookAt.x - horz * glm::sin(thetaR);
    position.z = lookAt.z - horz * glm::cos(thetaR);
    position.y = lookAt.y + vert;
}

void ThirdPersonCamera::takeMouseInput(const double  dx, const double dy) {
    Camera::takeMouseInput(dx, dy);
}

void ThirdPersonCamera::updateLookAt() {
    lookAt = *lookRef;
}

void ThirdPersonCamera::moveForward(const float timeStep) {
    glm::vec3 newPos = position + w * MOVE_SPEED * timeStep;
    if (glm::distance(newPos, lookAt) > MIN_DISTANCE) {
        position = newPos;
    }
}

void ThirdPersonCamera::moveBackward(const float timeStep) {
    glm::vec3 newPos = position - w * MOVE_SPEED * timeStep;
    if (glm::distance(newPos, lookAt) < MAX_DISTANCE) {
        position = newPos;
    }
}

void ThirdPersonCamera::moveLeft(const float timeStep) {
    theta -= MOVE_SPEED * timeStep;
}

void ThirdPersonCamera::moveRight(const float timeStep) {
    theta += MOVE_SPEED * timeStep;
}

void ThirdPersonCamera::moveUp(const float timeStep) {
    phi += MOVE_SPEED * timeStep;
}

void ThirdPersonCamera::moveDown(const float timeStep) {
    phi -= MOVE_SPEED * timeStep;
}
