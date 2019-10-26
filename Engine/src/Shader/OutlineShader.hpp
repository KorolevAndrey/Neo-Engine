#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

namespace neo {

    class OutlineShader : public Shader {

    public:

        OutlineShader() :
            Shader("Outline Shader",
                R"(
                layout(location = 0) in vec3 vertPos;
                uniform mat4 P, V, M;
                out vec2 fragTex;
                void main() {
                    gl_Position = P * V * M * vec4(vertPos, 1.0);
                })",
                R"(
                uniform vec4 outlineColor;
                out vec4 color;
                void main() {
                    color = outlineColor;
                })")
        {}

        virtual void render(const CameraComponent &camera) override {

            bind();

            CHECK_GL(glCullFace(GL_FRONT));

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            const auto cameraFrustum = camera.getGameObject().getComponentByType<FrustumComponent>();

            for (auto& renderable : Engine::getComponentTuples<renderable::OutlineRenderable, MeshComponent, SpatialComponent>()) {
                auto renderableOutline = renderable.get<renderable::OutlineRenderable>();
                auto renderableMesh = renderable.get<MeshComponent>();
                auto renderableSpatial = renderable.get<SpatialComponent>();

                // VFC
                if (cameraFrustum) {
                    if (const auto& boundingBox = renderable.gameObject.getComponentByType<BoundingBoxComponent>()) {
                        float radius = glm::max(glm::max(renderableSpatial->getScale().x, renderableSpatial->getScale().y), renderableSpatial->getScale().z) * boundingBox->getRadius();
                        if (!cameraFrustum->isInFrustum(renderableSpatial->getPosition(), radius)) {
                            continue;
                        }
                    }
                }

                /* Bind mesh */
                const Mesh & mesh(renderableMesh->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                glm::mat4 M = renderableSpatial->getModelMatrix() * glm::scale(glm::mat4(1.f), glm::vec3(1.f + renderableOutline->mScale));

                loadUniform("M", M);
                loadUniform("outlineColor", renderableOutline->mColor);

                /* DRAW */
                mesh.draw();
            }

            unbind();
        }
    };
}
