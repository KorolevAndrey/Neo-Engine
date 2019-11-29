#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "ext/bullet3/btBulletDynamicsCommon.h"

using namespace neo;

class BulletSystem : public System {

public:

    btAlignedObjectArray <btCollisionShape*> mCollisionShapes;
    btBroadphaseInterface* mBroadphase;
    btCollisionDispatcher* mDispatcher;
    btConstraintSolver* mSolver;
    btDefaultCollisionConfiguration* mCollisionConfiguration;
    btDynamicsWorld* mDynamicsWorld;
    std::vector<btRigidBody*> mBoxesRB;

    BulletSystem() :
        System("Bullet System")
    {}

    virtual void init() override {
        mCollisionConfiguration = new btDefaultCollisionConfiguration();
        mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
        mBroadphase = new btDbvtBroadphase();
        btSequentialImpulseConstraintSolver* sol = new
            btSequentialImpulseConstraintSolver;
        mSolver = sol;
        mDynamicsWorld = new
            btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver,
                mCollisionConfiguration);
        mDynamicsWorld->setGravity(btVector3(0, -10, 0));
        btBoxShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));
        mCollisionShapes.push_back(groundShape);
        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0, -50, 0));
        btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(btScalar(0), myMotionState, groundShape, btVector3(0, 0, 0));
        btRigidBody* body = new btRigidBody(rbInfo);
        mDynamicsWorld->addRigidBody(body);
        // ---
        const float SCALING = 0.5f;
        btBoxShape* colShape = new btBoxShape(btVector3(SCALING * 1, SCALING *
            1, SCALING * 1));
        mCollisionShapes.push_back(colShape);
        // ---
        btTransform startTransform;
        startTransform.setIdentity();
        btScalar mass(1.0f);
        btVector3 localInertia(0, 0, 0);
        colShape->calculateLocalInertia(mass, localInertia);
        for (int i = 0; i < 200; i++) {
            startTransform.setOrigin(SCALING*btVector3(
                btScalar(Util::genRandom(-5.f, 5.f)),
                btScalar(2.0*i + 10),
                btScalar(Util::genRandom(-5.f, 5.f))));
            btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState
                , colShape, localInertia);
            btRigidBody* body = new btRigidBody(rbInfo);
            mBoxesRB.push_back(body);
            mDynamicsWorld->addRigidBody(body);
        }
    }

    virtual void update(const float dt) override {
        if (mDynamicsWorld) {
            mDynamicsWorld->stepSimulation(dt);
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
        for (int j = 0; j < mCollisionShapes.size(); j++)
        {
            btCollisionShape* shape = mCollisionShapes[j];
            delete shape;
        }
        mCollisionShapes.clear();
        delete mDynamicsWorld;
        delete mSolver;
        delete mBroadphase;
        delete mDispatcher;
        delete mCollisionConfiguration;
    }

};
