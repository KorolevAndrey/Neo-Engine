#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    class FrustumComponent : public Component {
        public:
            FrustumComponent(GameObject *go) :
                Component(go)
            {}

            // Bounds
            glm::vec3 NearLeftBottom;
            glm::vec3 NearLeftTop;
            glm::vec3 NearRightBottom;
            glm::vec3 NearRightTop;
            glm::vec3 FarLeftBottom;
            glm::vec3 FarLeftTop;
            glm::vec3 FarRightBottom;
            glm::vec3 FarRightTop;

            // Planes
            glm::vec4 mLeft;
            glm::vec4 mRight;
            glm::vec4 mTop;
            glm::vec4 mBottom;
            glm::vec4 mNear;
            glm::vec4 mFar;

            // Test if an object is inside the frustum
            bool isInFrustum(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& boxMin, const glm::vec3& boxMax) {
                float radius = glm::distance(boxMax, boxMin) / 2.f;

                // True position is the scaled center of the mesh + transition
                glm::vec3 truePosition = boxMin + glm::normalize(boxMax - boxMin) * radius;
                truePosition *= scale;
                truePosition += position;
                return _distanceToPlane(mLeft, truePosition)   > -radius &&
                       _distanceToPlane(mRight, truePosition)  > -radius &&
                       _distanceToPlane(mBottom, truePosition) > -radius &&
                       _distanceToPlane(mTop, truePosition)    > -radius &&
                       _distanceToPlane(mNear, truePosition)   > -radius &&
                       _distanceToPlane(mFar, truePosition)    > -radius;
            }

        private:

            inline float _distanceToPlane(glm::vec4 plane, glm::vec3 position) {
                return plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
            }

    };
}
