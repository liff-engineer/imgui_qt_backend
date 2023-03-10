#pragma once
#include "imgui.h"

class QOpenGLWidget;
class QOpenGLWindow;
IMGUI_IMPL_API bool     ImGui_ImplQt_Init(QOpenGLWidget* window);
IMGUI_IMPL_API bool     ImGui_ImplQt_Init(QOpenGLWindow* window);
IMGUI_IMPL_API void     ImGui_ImplQt_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplQt_NewFrame();
