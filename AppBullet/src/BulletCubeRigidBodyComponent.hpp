#pragma once

#include "GameObject/GameObject.hpp"
#include "Component/Component.hpp"

#include "ext/bullet3/btBulletDynamicsCommon.h"

using namespace neo;

class BulletCubeRigidBodyComponent : public Component {
public:

    btBoxShape* colShape;
    btScalar mass;
    btVector3 localInertia;
    btTransform startTransform;
    
    btDefaultMotionState* myMotionState;

    btRigidBody* body;

    BulletCubeRigidBodyComponent(GameObject* go, float mass = 1.f, glm::vec3 inertia = glm::vec3(0.f)) :
        Component(go),
        mass(mass),
        localInertia(inertia.x, inertia.y, inertia.z)
    {
        startTransform.setIdentity();
    }

    virtual void init() {
        colShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
        colShape->calculateLocalInertia(mass, localInertia);

        myMotionState = new btDefaultMotionState(startTransform);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
        body = new btRigidBody(rbInfo);
    };

    virtual void kill() {
        delete colShape;
    };
};
