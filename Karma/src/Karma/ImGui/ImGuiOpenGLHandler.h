// Major and heavy modifications of the original Dear ImGUI's take on OpenGL3 implementation
// https://github.com/ravimohan1991/imgui/blob/07334fa9c67e6c7c827cc76c2ac4de419a1658c1/backends/imgui_impl_opengl3.h

#pragma once
#include "imgui.h"      // IMGUI_IMPL_API
#undef __gl_h_
#include "glad/glad.h"

namespace Karma
{
	class KARMA_API ImGuiOpenGLHandler
	{
	public:
		// Backend API
		static bool ImGui_ImplOpenGL3_Init(const char* glsl_version = NULL);
		static void ImGui_ImplOpenGL3_Shutdown();
		static void ImGui_ImplOpenGL3_NewFrame();
		static void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

		static void ImGui_ImplOpenGL3_SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height, GLuint vertex_array_object);
		static bool CheckShader(GLuint handle, const char* desc);
		static bool CheckProgram(GLuint handle, const char* desc);
		static void ImGui_ImplOpenGL3_RenderWindow(ImGuiViewport* viewport, void*);
		static void ImGui_ImplOpenGL3_InitPlatformInterface();
		static void ImGui_ImplOpenGL3_ShutdownPlatformInterface();

		// (Optional) Called by Init/NewFrame/Shutdown
		static bool ImGui_ImplOpenGL3_CreateFontsTexture();
		static void ImGui_ImplOpenGL3_DestroyFontsTexture();
		static bool ImGui_ImplOpenGL3_CreateDeviceObjects();
		static void ImGui_ImplOpenGL3_DestroyDeviceObjects();
	};
}
