#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Component/ModelComponent/LineMeshComponent.hpp"

#include "BulletSystem.hpp"

#include "ext/bullet3/btBulletDynamicsCommon.h"

using namespace neo;

class BulletShader : public Shader, btIDebugDraw {

public:
    BulletShader() :
        Shader("Bullet Shader",
            R"(
                    layout (location = 0) in vec3 vertPos;
                    uniform mat4 P, V, M;
                    void main() {
                        gl_Position = P * V * vec4(vertPos, 1);
                    })",
            R"(
                    uniform vec3 col;
                    out vec4 color;
                    void main() {
                        color = vec4(col, 1.0);
                    })"
        )
    {
        m = new Mesh(GL_LINE_STRIP);
        m->addVertexBuffer(VertexType::Position, 0, 3, { 0,0,0,0,0,0 });
    }

    Mesh* m;
    int m_debugMode = 0;

    virtual void render(const CameraComponent &camera) override {
        bind();

        CHECK_GL(glEnable(GL_LINE_SMOOTH));
        CHECK_GL(glDisable(GL_DEPTH_TEST));

        /* Load PV */
        loadUniform("P", camera.getProj());
        loadUniform("V", camera.getView());

        Engine::getSystem<BulletSystem>().mDynamicsWorld->setDebugDrawer(this);
        Engine::getSystem<BulletSystem>().mDynamicsWorld->debugDrawWorld();

        unbind();
    }

    virtual void imguiEditor() override {
        if (ImGui::RadioButton("No debug", m_debugMode == btIDebugDraw::DBG_NoDebug)) {
            setDebugMode(btIDebugDraw::DBG_NoDebug);
        }
        if (ImGui::RadioButton("Wireframe", m_debugMode == btIDebugDraw::DBG_DrawWireframe)) {
            setDebugMode(btIDebugDraw::DBG_DrawWireframe);
        }
        if (ImGui::RadioButton("AABB", m_debugMode == btIDebugDraw::DBG_DrawAabb)) {
            setDebugMode(btIDebugDraw::DBG_DrawAabb);
        }
    }


    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
        m->updateVertexBuffer(VertexType::Position, {
            from[0], from[1], from[2],
            to[0], to[1], to[2]
            });

        loadUniform("col", glm::vec3(color[0], color[1], color[2]));

        m->draw();
    }

    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {}

    virtual void reportErrorWarning(const char* warningString) {}

    virtual void draw3dText(const btVector3& location, const char* textString) {}

    virtual void setDebugMode(int debugMode) { m_debugMode = debugMode; }

    virtual int  getDebugMode() const { return m_debugMode; }

};