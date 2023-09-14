// Major and heavy modifications of the original Dear ImGUI's take on OpenGL3 implementation
// https://github.com/ravimohan1991/imgui/blob/07334fa9c67e6c7c827cc76c2ac4de419a1658c1/backends/imgui_impl_opengl3.h

#pragma once
#include "KarmaGui.h"      // IMGUI_IMPL_API
#include "Karma/Renderer/Scene.h"

#define KGGUI_IMPL_OPENGL_USE_VERTEX_ARRAY

//#include "glad/glad.h"

#ifdef _WIN64
typedef signed   long long int khronos_ssize_t;
#else
typedef signed   long  int     khronos_ssize_t;
#endif

typedef unsigned int GLuint;
typedef int GLint;

typedef khronos_ssize_t GLsizeiptr;


namespace Karma
{
	// Decal data
	struct MesaDecalData
	{
		GLuint DecalRef;
		KGTextureID DecalID;
		int width, height;
	};

	// OpenGL Data
	struct KarmaGui_ImplOpenGL3_Data
	{
		GLuint          GlVersion;               // Extracted at runtime using GL_MAJOR_VERSION, GL_MINOR_VERSION queries (e.g. 320 for GL 3.2)
		char            GlslVersionString[32];   // Specified by user or detected based on compile time GL settings.
		GLuint          FontTexture;
		// Images data
		std::vector<MesaDecalData>  openglMesaDecalDataList;
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
		//KarmaGui_ImplOpenGL3_Data() { memset((void*)this, 0, sizeof(*this)); }
	};

	// OpenGL vertex attribute state (for ES 1.0 and ES 2.0 only)
#ifndef KGGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
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

	class KARMA_API KarmaGuiOpenGLHandler
	{
	public:
		// Backend API
		static bool KarmaGui_ImplOpenGL3_Init(const char* glsl_version = NULL);
		static void KarmaGui_ImplOpenGL3_Shutdown();
		static void KarmaGui_ImplOpenGL3_NewFrame();
		static void KarmaGui_ImplOpenGL3_RenderDrawData(KGDrawData* draw_data);
		/*
		inline static KarmaGui_ImplOpenGL3_Data* KarmaGui_ImplOpenGL3_GetBackendData()
		{
			return KarmaGui::GetCurrentContext() ? (KarmaGui_ImplOpenGL3_Data*)KarmaGui::GetIO().BackendRendererUserData : NULL;
		}*/

		static void KarmaGui_ImplOpenGL3_SetupRenderState(KGDrawData* draw_data, int fb_width, int fb_height, GLuint vertex_array_object);
		static void KarmaGui_ImpOpenGL3_SetupRenderStateFor3DRendering(Scene* sceneToDraw, KGDrawData* drawData);
		static bool CheckShader(GLuint handle, const char* desc);
		static bool CheckProgram(GLuint handle, const char* desc);
		static void KarmaGui_ImplOpenGL3_RenderWindow(KarmaGuiViewport* viewport, void*);
		static void KarmaGui_ImplOpenGL3_InitPlatformInterface();
		static void KarmaGui_ImplOpenGL3_ShutdownPlatformInterface();
		static void KarmaGui_ImplOpenGL3_CreateTexture(char const* fileName, const std::string& label = "");

		// (Optional) Called by Init/NewFrame/Shutdown
		static bool KarmaGui_ImplOpenGL3_CreateFontsTexture();
		static void KarmaGui_ImplOpenGL3_DestroyFontsTexture();
		static bool KarmaGui_ImplOpenGL3_CreateDeviceObjects();
		static void KarmaGui_ImplOpenGL3_DestroyDeviceObjects();
	};
}
