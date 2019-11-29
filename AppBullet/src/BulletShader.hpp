#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

#include "BulletSystem.hpp"

using namespace neo;

class BulletShader : public Shader {

public:

    BulletShader(const std::string& vert, const std::string& frag) :
        Shader("Bullet Shader", vert, frag)
    {}

    virtual void render(const CameraComponent &camera) override {
        bind();

        loadUniform("P", camera.getProj());
        loadUniform("V", camera.getView());

                CHECK_GL(glDisable(GL_CULL_FACE));
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        auto& sys = Engine::getSystem<BulletSystem>();
        for (std::vector<btRigidBody*>::iterator it = sys.mBoxesRB.begin(); it != sys.mBoxesRB.end(); ++it) {
            btRigidBody* body = *it;
            btTransform worldTransform;
            body->getMotionState()->getWorldTransform(worldTransform);

            glm::mat4 M;
            worldTransform.getOpenGLMatrix(glm::value_ptr(M));
            loadUniform("M", M);

            Library::getMesh("cube")->draw();
            // gl::pushMatrices();
            // glMultMatrixf((GLfloat*)m);
            // gl::drawColorCube(Vec3f::zero(), Vec3f::one());
            // gl::popMatrices();
        }

        unbind();
    }
};
