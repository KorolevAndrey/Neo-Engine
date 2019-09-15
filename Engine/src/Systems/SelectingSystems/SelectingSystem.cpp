#include <Engine.hpp>
#include "SelectingSystem.hpp"

namespace neo {

    void SelectingSystem::init() {
        // Operate on unselected objects
        for (auto selectable : Engine::getComponents<SelectableComponent>()) {
            mResetOperation(selectable);
        }
    }

    void SelectingSystem::update(const float dt) {
        if (auto mouseRay = Engine::getSingleComponent<MouseRayComponent>()) {
            // Decide to remove unselected objects
            for (auto selected : Engine::getComponents<SelectedComponent>()) {
                if (mRemoveDecider(selected)) {
                    Engine::removeComponent<SelectedComponent>(*selected);
                }
            }


            // Select a new object
            for (auto selectable : Engine::getComponents<SelectableComponent>()) {
                if (auto bb = selectable->getGameObject().getComponentByType<BoundingBoxComponent>()) {
                    float maxDistance = mMaxDist;
                    if (auto camera = Engine::getSingleComponent<CameraComponent>()) {
                        maxDistance = glm::max(maxDistance, camera->getNearFar().y);
                    }

                    // Ray march
                    for (float i = 0.f; i < maxDistance; i += maxDistance / static_cast<float>(mMaxMarches)) {
                        if (bb->intersect(mouseRay->position + mouseRay->direction * i)) {
                            // Add new selected
                            Engine::addComponent<SelectedComponent>(&selectable->getGameObject());
                            // Operate on selected objects
                            for (auto selected : Engine::getComponents<SelectedComponent>()) {
                                mSelectOperation(selected);
                            }

                        }
                    }
                }
            }

            // Operate on unselected objects
            for (auto selectable : Engine::getComponents<SelectableComponent>()) {
                if (!selectable->getGameObject().getComponentByType<SelectedComponent>()) {
                    mResetOperation(selectable);
                }
            }
        }
    }

    void SelectingSystem::imguiEditor() {
        auto selected = Engine::getComponents<SelectedComponent>();
        if (selected.size()) {
            mEditorOperation(selected);
        }
    }

}
