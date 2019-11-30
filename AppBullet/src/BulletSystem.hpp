#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "BulletCubeRigidBodyComponent.hpp"
#include "RegisterBulletComponent.hpp"

#include "ext/bullet3/btBulletDynamicsCommon.h"

using namespace neo;

void myProfilerEnter(const char* msg) {
    MICROPROFILE_DEFINE(Bullet, "BulletSystem", msg, MP_AUTO);
    MICROPROFILE_ENTER(Bullet);
}

void myProfilerLeave() {
    MICROPROFILE_LEAVE();
}


class BulletSystem : public System {

public:

    btBroadphaseInterface* mBroadphase;
    btCollisionDispatcher* mDispatcher;
    btConstraintSolver* mSolver;
    btDefaultCollisionConfiguration* mCollisionConfiguration;
    btDynamicsWorld* mDynamicsWorld;

    BulletSystem() :
        System("Bullet System")
    {}

    virtual void init() override {
        btSetCustomEnterProfileZoneFunc(myProfilerEnter);
        btSetCustomLeaveProfileZoneFunc(myProfilerLeave);

        mCollisionConfiguration = new btDefaultCollisionConfiguration();
        mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
        mBroadphase = new btDbvtBroadphase();
        btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
        mSolver = sol;
        mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfiguration);
        mDynamicsWorld->setGravity(btVector3(0, -4, 0));
    }

    virtual void update(const float dt) override {
        /* Init new components */
        MICROPROFILE_ENTERI("BulletSystem", "Register bodies", MP_AUTO);
        for (auto& comp : Engine::getComponents<RegisterBulletComponent>()) {
            auto spatial = comp->getGameObject().getComponentByType<SpatialComponent>();
            NEO_ASSERT(spatial, "Attempting to register a bullet body without a SpatialComponent");

            if (auto cube = comp->getGameObject().getComponentByType<BulletCubeRigidBodyComponent>()) {
                cube->startTransform.setOrigin(btVector3(
                    spatial->getPosition().x,
                    spatial->getPosition().y,
                    spatial->getPosition().z
                ));
                cube->colShape->setLocalScaling(btVector3(
                    spatial->getScale().x,
                    spatial->getScale().y,
                    spatial->getScale().z
                ));
                cube->myMotionState->setWorldTransform(cube->startTransform);
                cube->body->setMotionState(cube->myMotionState);
                mDynamicsWorld->addRigidBody(cube->body);
            }

            Engine::removeComponent<RegisterBulletComponent>(*comp);
        }
        MICROPROFILE_LEAVE();

        /* Run simulation */
        MICROPROFILE_ENTERI("BulletSystem", "Bullet3::stepSim", MP_AUTO);
        mDynamicsWorld->stepSimulation(dt);
        MICROPROFILE_LEAVE();

        /* Update spatials */
        MICROPROFILE_ENTERI("BulletSystem", "Update spatials", MP_AUTO);
        for (auto& cube : Engine::getComponentTuples<BulletCubeRigidBodyComponent, SpatialComponent>()) {
            btTransform worldTransform;
            cube->get<BulletCubeRigidBodyComponent>()->body->getMotionState()->getWorldTransform(worldTransform);

            glm::mat4 M;
            worldTransform.getOpenGLMatrix(glm::value_ptr(M));
            cube->get<SpatialComponent>()->setPosition(glm::vec3(M[3][0], M[3][1], M[3][2]));
            cube->get<SpatialComponent>()->setOrientation(glm::mat3(M));
        }
        MICROPROFILE_LEAVE();
    }

    virtual void shutdown() override {
        for (int i = mDynamicsWorld->getNumCollisionObjects() - 1; i >= 0
            ; i--)
        {
            btCollisionObject* obj = mDynamicsWorld->getCollisionObjectArray()
                [i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState()) {
                delete body->getMotionState();
            }
            mDynamicsWorld->removeCollisionObject(obj);
            delete obj;
        }
        delete mDynamicsWorld;
        delete mSolver;
        delete mBroadphase;
        delete mDispatcher;
        delete mCollisionConfiguration;
    }

};
