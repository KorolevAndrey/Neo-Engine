#pragma once

#include "NeoEngine.hpp"

#include "CustomRenderable.hpp"
#include "Shader/Shader.hpp"
#include "Shader/GLHelper.hpp"

using namespace neo;

class WireShader : public Shader {

    public:
        WireShader(const std::string &res, const std::string &vert, const std::string &frag) :
            Shader(res, vert, frag)
        {}

        virtual void render(float dt, const CameraComponent *camera) override {
            bind();

            /* Load PV */
            loadMatrix(getUniform("P"), camera->getProj());
            loadMatrix(getUniform("V"), camera->getView());

            auto renderables = NeoEngine::getComponents<CustomRenderable>();
            for (auto r : renderables) {
                /* Bind mesh */
                const Mesh & mesh(*r->getMesh());
                CHECK_GL(glBindVertexArray(mesh.vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                loadMatrix(getUniform("M"), r->getGameObject().getSpatial()->getModelMatrix());

                /* Draw outline */
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
                CHECK_GL(glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr));
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            unbind();
        }
};