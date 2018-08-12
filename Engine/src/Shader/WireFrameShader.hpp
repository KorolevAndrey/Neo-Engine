#pragma once

#include "Shader/Shader.hpp"
#include "Util/GLHelper.hpp"

namespace neo {

    class WireframeShader : public Shader {

        public:
            WireframeShader(std::string res) :
                Shader("Wire Shader",
                    _strdup("\
                        #version 330 core\n\
                        layout (location = 0) in vec3 vertPos;\
                        uniform mat4 P, V, M;\
                        void main() {\
                            gl_Position = P * V * M * vec4(vertPos, 1);\
                        }"),
                    _strdup("\
                        #version 330 core\n\
                        out vec4 color;\
                        void main() {\
                            color = vec4(1.0);\
                        }")
                )
            {}

            virtual void render(float dt, const RenderSystem &renderSystem) override {
                bind();

                /* Load PV */
                const std::vector<CameraComponent *> cameras = NeoEngine::getComponents<CameraComponent>();
                if (cameras.size()) {
                    loadMatrix(getUniform("P"), cameras.at(0)->getProj());
                    loadMatrix(getUniform("V"), cameras.at(0)->getView());
                }

                for (auto r : renderSystem.getRenderables<WireframeShader, RenderableComponent>()) {
                    /* Bind mesh */
                    const Mesh & mesh(r->getMesh());
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

}