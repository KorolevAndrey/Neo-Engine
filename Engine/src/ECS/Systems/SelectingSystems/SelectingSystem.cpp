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
        auto mouseRay = Engine::getSingleComponent<MouseRayComponent>();
        auto mainCamera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>();
        if (!mouseRay || !mainCamera) {
            return;
        }

        float intersectDist = 0.f;
        float maxDistance = glm::min(mMaxDist, mainCamera->get<CameraComponent>()->getNearFar().y);

        // Select a new object
        SelectableComponent* selectedSelectable = nullptr;
        for (auto& selectable : Engine::getComponentTuples<SelectableComponent, BoundingBoxComponent, SpatialComponent>()) {
            auto selectableSpatial = selectable->get<SpatialComponent>();
            auto selectableBox = selectable->get<BoundingBoxComponent>();

            // Frustum culling
            if (const auto& frustumPlanes = mainCamera->mGameObject.getComponentByType<FrustumComponent>()) {
                if (!frustumPlanes->isInFrustum(selectableSpatial->getPosition(), selectableSpatial->getScale(), selectableBox->mMin, selectableBox->mMax)) {
                    continue;
                }
            }

            // Ray march
            for (float i = 0.f; i <= maxDistance; i += maxDistance / static_cast<float>(mMaxMarches)) {
                if (selectableBox->intersect(mouseRay->mPosition + mouseRay->mDirection * i)) {
                    selectedSelectable = selectable->get<SelectableComponent>();
                    intersectDist = i;
                    break;
                }
            }
            if (selectedSelectable) {
                break;
            }
        }

        SelectedComponent* selected = nullptr;
        if (selectedSelectable) {
            selected = selectedSelectable->getGameObject().getComponentByType<SelectedComponent>();
            if (!selected) {
                selected = &Engine::addComponent<SelectedComponent>(&selectedSelectable->getGameObject());
                mSelectOperation(selected, mouseRay, intersectDist);
            }
        }

        // Decide to remove unselected objects
        for (auto eSelected : Engine::getComponents<SelectedComponent>()) {
            if (mRemoveDecider(eSelected) && (!selected || (&eSelected->getGameObject() != &selected->getGameObject()))) {
                Engine::removeComponent<SelectedComponent>(*eSelected);
            }
        }

        // Operate on unselected objects
        for (auto selectable : Engine::getComponents<SelectableComponent>()) {
            if (!selectable->getGameObject().getComponentByType<SelectedComponent>()) {
                mResetOperation(selectable);
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
