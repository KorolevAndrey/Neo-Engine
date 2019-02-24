#include "SpatialComponent.hpp"

#include "Messaging/Messenger.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    SpatialComponent::SpatialComponent(GameObject *go) :
        Component(go),
        Orientable(),
        mPosition(0.f),
        mScale(1.f),
        mModelMatrix(),
        mNormalMatrix(),
        mModelMatrixDirty(true),
        mNormalMatrixDirty(true)
    {}

    SpatialComponent::SpatialComponent(GameObject *go, const glm::vec3 & p) :
        SpatialComponent(go) {
        mPosition = p;
    }

    SpatialComponent::SpatialComponent(GameObject *go, const glm::vec3 & p, const glm::vec3 & s) :
        SpatialComponent(go, p) {
        mScale = s;
    }

    SpatialComponent::SpatialComponent(GameObject *go, const glm::vec3 & p, const glm::vec3 & s, const glm::mat3 & o) :
        SpatialComponent(go, p, s) {
        setOrientation(o);
    }

    void SpatialComponent::move(const glm::vec3 & delta) {
        if (glm::length(delta) == 0.f) {
            return;
        }

        mPosition += delta;
        mModelMatrixDirty = true;
        Messenger::sendMessage<SpatialChangeMessage>(mGameObject, *this);
    }

    void SpatialComponent::resize(const glm::vec3 & factor) {
        if (factor == glm::vec3(1.f)) {
            return;
        }

        mScale *= factor;
        mModelMatrixDirty = true;
        mNormalMatrixDirty = true;
        Messenger::sendMessage<SpatialChangeMessage>(mGameObject, *this);
    }

    void SpatialComponent::rotate(const glm::mat3 & mat) {
        Orientable::rotate(mat);
        mModelMatrixDirty = true;
        mNormalMatrixDirty = true;
        Messenger::sendMessage<SpatialChangeMessage>(mGameObject, *this);
    }

    void SpatialComponent::setPosition(const glm::vec3 & loc) {
        if (mPosition == loc) {
            return;
        }

        mPosition = loc;
        mModelMatrixDirty = true;
        Messenger::sendMessage<SpatialChangeMessage>(mGameObject, *this);
    }

    void SpatialComponent::setScale(const glm::vec3 & scale) {
        if (this->mScale == scale) {
            return;
        }

        this->mScale = scale;
        mModelMatrixDirty = true;
        mNormalMatrixDirty = true;
        Messenger::sendMessage<SpatialChangeMessage>(mGameObject, *this);
    }

    void SpatialComponent::setOrientation(const glm::mat3 & orient) {
        Orientable::setOrientation(orient);
        mModelMatrixDirty = true;
        mNormalMatrixDirty = true;
        Messenger::sendMessage<SpatialChangeMessage>(mGameObject, *this);
    }

    void SpatialComponent::setUVW(const glm::vec3 & u, const glm::vec3 & v, const glm::vec3 & w) {
        Orientable::setUVW(u, v, w);
        mModelMatrixDirty = true;
        mNormalMatrixDirty = true;
        Messenger::sendMessage<SpatialChangeMessage>(mGameObject, *this);
    }
        
    const glm::mat4 & SpatialComponent::getModelMatrix() const {
        if (mModelMatrixDirty) {
            _detModelMatrix();
        }
        return mModelMatrix;
    }

    const glm::mat3 & SpatialComponent::getNormalMatrix() const {
        if (mNormalMatrixDirty) {
            _detNormalMatrix();
        }
        return mNormalMatrix;
    }

    void SpatialComponent::_detModelMatrix() const {
        mModelMatrix = glm::scale(glm::translate(glm::mat4(), mPosition) * glm::mat4(getOrientation()), mScale);
        mModelMatrixDirty = false;
    }

    void SpatialComponent::_detNormalMatrix() const {
        if (mScale.x == mScale.y && mScale.y == mScale.z) {
            mNormalMatrix = glm::mat3(mModelMatrix);
        }
        else {
            mNormalMatrix = getOrientation() * glm::mat3(glm::scale(glm::mat4(), 1.0f / mScale));
        }
        mNormalMatrixDirty = false;
    }
}
