#include <Engine.hpp>

#include "FrustumToLineSystem.hpp"

namespace neo {

    void FrustumToLineSystem::update(const float dt) {
        for (auto camera : Engine::getComponents<CameraComponent>()) {
            if (auto line = camera->getGameObject().getComponentByType<LineMeshComponent>()) {
                if (auto bounds = camera->getGameObject().getComponentByType<FrustumComponent>()) {

                    line->clearNodes();

                    // We have to do this ridiculousness because line strip (:
                    line->addNode(bounds->NearLeftBottom);
                    line->addNode(bounds->NearLeftTop);
                    line->addNode(bounds->NearRightTop);
                    line->addNode(bounds->NearRightBottom);
                    line->addNode(bounds->NearLeftBottom);
                    line->addNode(bounds->FarLeftBottom);
                    line->addNode(bounds->FarLeftTop);
                    line->addNode(bounds->NearLeftTop);
                    line->addNode(bounds->FarLeftTop);
                    line->addNode(bounds->FarRightTop);
                    line->addNode(bounds->NearRightTop);
                    line->addNode(bounds->FarRightTop);
                    line->addNode(bounds->FarRightBottom);
                    line->addNode(bounds->NearRightBottom);
                    line->addNode(bounds->FarRightBottom);
                    line->addNode(bounds->FarLeftBottom);
                }

            }
        }

    }
}
