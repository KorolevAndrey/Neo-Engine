#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

namespace neo {

    class PhongShader : public Shader {

    public:

        PhongShader() :
            Shader("Phong Shader",
                R"(
                layout(location = 0) in vec3 vertPos;
                layout(location = 1) in vec3 vertNor;
                layout(location = 2) in vec2 vertTex;
                uniform mat4 P, V, M;
                uniform mat3 N;
                out vec4 fragPos;
                out vec3 fragNor;
                out vec2 fragTex;
                void main() {
                    fragPos = M * vec4(vertPos, 1.0);
                    fragNor = N * vertNor;
                    fragTex = vertTex;
                    gl_Position = P * V * fragPos;
                })",
                R"(
                #include "phong.glsl"
                #include "alphaDiscard.glsl"

                in vec4 fragPos;
                in vec3 fragNor;
                in vec2 fragTex;
                uniform sampler2D diffuseMap;
                uniform bool useTexture;
                uniform float ambient;
                uniform vec3 diffuseColor;
                uniform vec3 specularColor;
                uniform float shine;
                uniform vec3 camPos;
                uniform vec3 lightPos;
                uniform vec3 lightCol;
                uniform vec3 lightAtt;
                out vec4 color;
                void main() {
                    vec4 albedo = vec4(diffuseColor, 1.f);
                    if (useTexture) {
                        albedo = texture(diffuseMap, fragTex);
                        alphaDiscard(albedo.a);
                    }
                    color.rgb = albedo.rgb * ambient + 
                                getPhong(fragNor, fragPos.rgb, camPos, lightPos, lightAtt, lightCol, albedo.rgb, specularColor, shine);
                    color.a = albedo.a;
                })")
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

            /* Load light */
            if (const auto light = Engine::getSingleComponent<LightComponent>()) {
                loadUniform("lightPos", light->getGameObject().getSpatial()->getPosition());
                loadUniform("lightCol", light->mColor);
                loadUniform("lightAtt", light->mAttenuation);
            }

            for (const auto& renderable : Engine::getComponents<renderable::PhongRenderable>()) {
                const auto& meshComponent = renderable->getGameObject().getComponentByType<MeshComponent>();
                if (!meshComponent) {
                    continue;
                }

                const auto& renderableSpatial = renderable->getGameObject().getSpatial();

                // VFC
                if (const auto& boundingBox = renderable->getGameObject().getComponentByType<BoundingBoxComponent>()) {
                    if (const auto& frustumPlanes = camera.getGameObject().getComponentByType<FrustumPlanesComponent>()) {
                        // todo - this is broke
                        // todo - can radius be a vec3?
                        float radius = glm::distance(boundingBox->mMax, boundingBox->mMin) * glm::max(glm::max(renderableSpatial->getScale().x, renderableSpatial->getScale().y), renderableSpatial->getScale().z) * 0.5f;
                        if (!frustumPlanes->isInFrustum(renderableSpatial->getPosition(), radius)) {
                            continue;
                    }
                }
            }

                /* Bind mesh */
                const Mesh & mesh(meshComponent->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                loadUniform("M", renderableSpatial->getModelMatrix());
                loadUniform("N", renderableSpatial->getNormalMatrix());

                /* Bind texture */
                if (auto diffuseMap = renderable->getGameObject().getComponentByType<DiffuseMapComponent>()) {
                    auto texture = (const Texture2D *)(diffuseMap->mTexture);
                    texture->bind();
                    loadUniform("diffuseMap", texture->mTextureID);
                    loadUniform("useTexture", true);
                }
                else {
                    loadUniform("useTexture", false);
                }

                /* Bind material */
                if (auto matComp = renderable->getGameObject().getComponentByType<MaterialComponent>()) {
                    loadUniform("ambient", matComp->mAmbient);
                    loadUniform("diffuseColor", matComp->mDiffuse);
                    loadUniform("specularColor", matComp->mSpecular);
                    loadUniform("shine", matComp->mShine);
                }

                /* DRAW */
                mesh.draw();
            }

            unbind();
        }
    };
}
