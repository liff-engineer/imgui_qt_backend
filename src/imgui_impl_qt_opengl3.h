#pragma once

#include "imgui.h"

IMGUI_IMPL_API bool ImGui_ImplQtOpenGL3_Init(const char* glsl_version = nullptr);
IMGUI_IMPL_API void ImGui_ImplQtOpenGL3_Shutdown();
IMGUI_IMPL_API void ImGui_ImplQtOpenGL3_NewFrame();
IMGUI_IMPL_API void ImGui_ImplQtOpenGL3_RenderDrawData(ImDrawData* draw_data);

IMGUI_IMPL_API bool ImGui_ImplQtOpenGL3_CreateFontsTexture();
IMGUI_IMPL_API void ImGui_ImplQtOpenGL3_DestoryFontsTexture();
IMGUI_IMPL_API bool ImGui_ImplQtOpenGL3_CreateDeviceObjects();
IMGUI_IMPL_API void ImGui_ImplQtOpenGL3_DestoryDeviceObjects();
