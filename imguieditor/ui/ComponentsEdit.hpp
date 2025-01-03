#pragma once

#include "../UI.hpp"

#include <engine/NewResourceManager.hpp>
#include <engine/components/Components.hpp>

namespace ui {

void componentEdit(EditorContext &editorContext, engine::components::Transform &transform);
void componentEdit(EditorContext &editorContext, engine::components::Material &material);
inline void componentEdit(EditorContext&, auto&) {}

} // namespace ui
