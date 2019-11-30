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
                auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>();
                glm::vec3 d = camera->get<SpatialComponent>()->getLookDir();
                cube->startTransform.setOrigin(btVector3(
                    spatial->getPosition().x + 3.2 * d.x + Util::genRandom(-0.5f, 0.5f),
                    spatial->getPosition().y + 3.2 * d.y + Util::genRandom(-0.5f, 0.5f),
                    spatial->getPosition().z + 3.2 * d.z + Util::genRandom(-0.5f, 0.5f)
                ));
                cube->colShape->setLocalScaling(btVector3(
                    spatial->getScale().x,
                    spatial->getScale().y,
                    spatial->getScale().z
                ));
                cube->myMotionState->setWorldTransform(cube->startTransform);
                cube->body->setMotionState(cube->myMotionState);
                cube->body->setFriction(2.f);
                mDynamicsWorld->addRigidBody(cube->body);

                cube->body->applyCentralForce(btVector3(500 * d.x, 500 * d.y, 500 * d.z));
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

        static float t = 0.f;
        t += dt;

        if (t > 0.1f) {
            if (Keyboard::isKeyPressed(GLFW_KEY_V)) {
                auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>();
                auto gameObject = &Engine::createGameObject();
                Engine::addComponent<MeshComponent>(gameObject, Library::getMesh("cube"));
                Engine::addComponent<SpatialComponent>(gameObject, camera->get<SpatialComponent>()->getPosition(), Util::genRandomVec3());
                Engine::addComponent<renderable::PhongRenderable>(gameObject);
                Engine::addComponent<MaterialComponent>(gameObject, 0.2f, Util::genRandomVec3());
                Engine::addComponent<RegisterBulletComponent>(gameObject);
                auto& rb = Engine::addComponent<BulletCubeRigidBodyComponent>(gameObject);
                rb.startTransform.setRotation(btQuaternion(Util::genRandom(0.f, 360.f), Util::genRandom(0.f, 360.f), Util::genRandom(0.f, 360.f)));

                t = 0.f;
            }
        }
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
