// Major and heavy modifications of the original Dear ImGUI's take on OpenGL3 implementation
// https://github.com/ravimohan1991/imgui/blob/07334fa9c67e6c7c827cc76c2ac4de419a1658c1/backends/imgui_impl_opengl3.h

#pragma once
#include "imgui.h"      // IMGUI_IMPL_API
#undef __gl_h_
#include "glad/glad.h"

namespace Karma
{
	// Decal data
	struct MesaDecalData
	{
		GLuint DecalRef;
		ImTextureID DecalID;
		int width, height;
	};

	// OpenGL Data
	struct ImGui_ImplOpenGL3_Data
	{
		GLuint          GlVersion;               // Extracted at runtime using GL_MAJOR_VERSION, GL_MINOR_VERSION queries (e.g. 320 for GL 3.2)
		char            GlslVersionString[32];   // Specified by user or detected based on compile time GL settings.
		GLuint          FontTexture;
		// Images data
		std::vector<MesaDecalData>  mesaDecalDataList;
		GLuint          ShaderHandle;
		GLint           AttribLocationTex;       // Uniforms location
		GLint           AttribLocationProjMtx;
		GLuint          AttribLocationVtxPos;    // Vertex attributes location
		GLuint          AttribLocationVtxUV;
		GLuint          AttribLocationVtxColor;
		unsigned int    VboHandle, ElementsHandle;
		GLsizeiptr      VertexBufferSize;
		GLsizeiptr      IndexBufferSize;
		bool            HasClipOrigin;
		bool            UseBufferSubData;
		ImGui_ImplOpenGL3_Data() { memset((void*)this, 0, sizeof(*this)); }
	};

	// OpenGL vertex attribute state (for ES 1.0 and ES 2.0 only)
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
	struct ImGui_ImplOpenGL3_VtxAttribState
	{
		GLint   Enabled, Size, Type, Normalized, Stride;
		GLvoid* Ptr;

		void GetState(GLint index)
		{
			glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &Enabled);
			glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_SIZE, &Size);
			glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_TYPE, &Type);
			glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &Normalized);
			glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &Stride);
			glGetVertexAttribPointerv(index, GL_VERTEX_ATTRIB_ARRAY_POINTER, &Ptr);
		}
		void SetState(GLint index)
		{
			glVertexAttribPointer(index, Size, Type, (GLboolean)Normalized, Stride, Ptr);
			if (Enabled) glEnableVertexAttribArray(index); else glDisableVertexAttribArray(index);
		}
	};
#endif

	class KARMA_API ImGuiOpenGLHandler
	{
	public:
		// Backend API
		static bool ImGui_ImplOpenGL3_Init(const char* glsl_version = NULL);
		static void ImGui_ImplOpenGL3_Shutdown();
		static void ImGui_ImplOpenGL3_NewFrame();
		static void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);
		// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
		// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
		inline static ImGui_ImplOpenGL3_Data* ImGui_ImplOpenGL3_GetBackendData()
		{
			return ImGui::GetCurrentContext() ? (ImGui_ImplOpenGL3_Data*)ImGui::GetIO().BackendRendererUserData : NULL;
		}

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