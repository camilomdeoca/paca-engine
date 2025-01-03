#pragma once

#include "../UI.hpp"
#include "engine/components/Skybox.hpp"

#include <engine/NewResourceManager.hpp>
#include <engine/components/Components.hpp>

namespace ui {

void componentEdit(EditorContext &editorContext, engine::components::Transform &transform);
void componentEdit(EditorContext &editorContext, engine::components::Material &material);
void componentEdit(EditorContext &editorContext, engine::components::StaticMesh &staticMesh);
void componentEdit(EditorContext &editorContext, engine::components::AnimatedMesh &animatedMesh);
void componentEdit(EditorContext &editorContext, engine::components::PointLight &pointLight);
void componentEdit(EditorContext &editorContext, engine::components::DirectionalLight &directionalLight);
void componentEdit(EditorContext &editorContext, engine::components::Skybox &skybox);
inline void componentEdit(EditorContext&, auto&) {}

} // namespace ui
