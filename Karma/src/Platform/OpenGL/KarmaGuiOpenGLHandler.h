/**
 * @file KarmaGuiOpenGLHandler.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains KarmaGuiOpenGLHandler class and relevant data structures.
 * @version 1.0
 * @date December 15, 2023
 *
 * @copyright 2014-2023 Omar Cornut
 */
// Dear ImGui (1.89.2) is Copyright (c) 2014-2023 Omar Cornut.

// Major and heavy modifications of the original Dear ImGUI's take on OpenGL3 implementation
// https://github.com/ravimohan1991/imgui/blob/07334fa9c67e6c7c827cc76c2ac4de419a1658c1/backends/imgui_impl_opengl3.h
// Disclaimer:
// Most of the member variables and function naming in the herein is credited to the author(s) of Dear ImGUI. I take no responsibility
// of damage caused by short names herein.

#pragma once
#include "KarmaGui.h"
#include "Karma/Renderer/Scene.h"

#define KGGUI_IMPL_OPENGL_USE_VERTEX_ARRAY

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
	/**
	 * @brief A data structure for holding information about Mesa texture. For instance, file and folder icons of Content Browser.
	 *
	 * @since Karma 1.0.0
	 */
	struct MesaDecalData
	{
		/**
		 * @brief An array in which the generated texture names, for decals, are stored.
		 *
		 * @see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGenTextures.xhtml
		 * @since Karma 1.0.0
		 */
		GLuint DecalRef;

		/**
		 * @brief A typecasted version of DecalRef
		 * The DecalID is used in KarmaGui::ImageButton.
		 *
		 * @note Different from DecalRef because used differently
		 * @since Karma 1.0.0
		 */
		KGTextureID DecalID;

		/**
		 * @brief 2D width of the decal
		 *
		 * @since Karma 1.0.0
		 */
		int width;

		/**
		 * @brief 2D height of the decal
		 *
		 * @since Karma 1.0.0
		 */
		int height;
	};

	/**
	 * @brief A data structure for KarmaGui's OpenGL backend renderer
	 *
	 * @note Accessible via KarmaGuiRenderer::GetBackendRendererUserData()
	 * @since Karma 1.0.0
	 */
	struct KarmaGui_ImplOpenGL3_Data
	{
		/**
		 * @brief The OpenGL version in use
		 *
		 * @note Extracted at runtime using GL_MAJOR_VERSION, GL_MINOR_VERSION queries (e.g. 320 for GL 3.2)
		 * @since Karma 1.0.0
		 */
		GLuint          GlVersion;

		/**
		 * @brief Specified by user or detected based on compile time GL settings
		 *
		 * @since Karma 1.0.0
		 */
		char            GlslVersionString[32];

		/**
		 * @brief Array ID for OpenGL's font texture
		 *
		 * @note Specifies an array in which the generated texture names are stored
		 *
		 * @see KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateFontsTexture()
		 * @since Karma 1.0.0
		 */
		GLuint          FontTexture;

		/**
		 * @brief The list of all the images' data to be used in KarmaGui with OpenGL api
		 *
		 * @note Usually used with KarmaGui_ImplOpenGL3_CreateTexture
		 * @since Karma 1.0.0
		 */
		std::vector<MesaDecalData>  openglMesaDecalDataList;

		/**
		 * @brief The handle for KarmaGui's shader program
		 *
		 * @since Karma 1.0.0
		 */
		GLuint          ShaderHandle;

		/**
		 * @brief An integer that represents the location of a texture uniform variable within a program object
		 *
		 * @since Karma 1.0.0
		 */
		GLint           AttribLocationTex;

		/**
		 * @brief An integer that represents the location of a projection matrix uniform variable within a program object
		 *
		 * @since Karma 1.0.0
		 */
		GLint           AttribLocationProjMtx;

		/**
		 * @brief An integer that represents the location of a vertex position (model transform?) uniform variable within a program object
		 *
		 * @since Karma 1.0.0
		 */
		GLuint          AttribLocationVtxPos;

		/**
		 * @brief An integer that represents the location of a UV uniform variable within a program object
		 *
		 * @since Karma 1.0.0
		 */
		GLuint          AttribLocationVtxUV;

		/**
		 * @brief An integer that represents the location of a vertex colo(u)r uniform variable within a program object
		 *
		 * @since Karma 1.0.0
		 */
		GLuint          AttribLocationVtxColor;

		/**
		 * @brief The handle for vertex buffer (GL_ARRAY_BUFFER) object meant for KarmaGui using OpenGL api
		 *
		 * @see ElementsHandle
		 * @since Karma 1.0.0
		 */
		unsigned int    VboHandle;

		/**
		 * @brief The handle for index buffer (GL_ELEMENT_ARRAY_BUFFER) object meant for KarmaGui using OpenGL api
		 *
		 * @note GL_ELEMENT_ARRAY_BUFFER is used to indicate the buffer you're presenting contains the indices of each element in the "other" (GL_ARRAY_BUFFER) buffer.
		 * @since Karma 1.0.0
		 */
		unsigned int ElementsHandle;

		/**
		 * @brief The size of total sum of vertices in int units of sizeof(KGDrawVert) to be utilized by KarmaGui using OpenGL api
		 *
		 * @since Karma 1.0.0
		 */
		GLsizeiptr      VertexBufferSize;

		/**
		 * @brief The size of total sum of indices in int units of sizeof(KGDrawVert) to be utilized by KarmaGui using OpenGL api
		 *
		 * @since Karma 1.0.0
		 */
		GLsizeiptr      IndexBufferSize;

		// Detect extensions we support
		/**
		 * @brief If extension GL_ARB_clip_control is supported and macro IMGUI_IMPL_OPENGL_MAY_HAVE_EXTENSIONS is defined or GlVersion >= 450 (setting the value true), then
		 * we look for clip origin. Assumption (in KarmaGui_ImplOpenGL3_SetupRenderState) is: origin is lower left.
		 *
		 * @note GL_CLIP_ORIGIN (the clip volume origin, set with glClipControl). Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
		 * @since Karma 1.0.0
		 */
		bool            HasClipOrigin;

		/**
		 * @brief True when the processor vendor is Intel and we are on Windows platform. When true glBufferSubData (instead of glBufferData) is used to upload vertex/index buffers.
		 *
		 * @since Karma 1.0.0
		 */
		bool            UseBufferSubData;

		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
		KarmaGui_ImplOpenGL3_Data()
		{
			//memset((void*)this, 0, sizeof(*this));
			VertexBufferSize = IndexBufferSize = 0;
		}
	};

	// OpenGL vertex attribute state (for ES 1.0 and ES 2.0 only)
#ifndef KGGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
	/**
	 * @brief A data structure for use in mobile and such devices. Mabe useful for future.
	 *
	 * @note I am not providing complete documentation since I don't have practical idea how they (mobile devices) are supposed to have implementation, yet.
	 * @since Karma 1.0.0
	 */
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

	/**
	 * @brief The chief class for KarmaGui's OpenGL based backend renderer.
	 *
	 * @since Karma 1.0.0
	 */
	class KARMA_API KarmaGuiOpenGLHandler
	{
	public:
		///////////////////////////////////////
		// Backend API
		///////////////////////////////////////
		/**
		 * @brief Initializing OpenGL backend renderer
		 *
		 * The following steps are taken:
		 * 1. KarmaGuiContext's members, chiefly BackendRendererUserData and flags, are set
		 * 2. Based on KGGuiConfigFlags_ViewportsEnable, drag and drop feature is set up by calling KarmaGui_ImplOpenGL3_InitPlatformInterface()
		 *
		 * @param glsl_version						The version of OpenGL driver being used
		 * @since Karma 1.0.0
		 */
		static bool KarmaGui_ImplOpenGL3_Init(const char* glsl_version = NULL);

		/**
		 * @brief KarmaGuiPlatformIO 's instance's member, platform_io.Renderer_RenderWindow, is set to KarmaGui_ImplOpenGL3_RenderWindow
		 *
		 * @note There is one global instance of KarmaGuiPlatformIO, in this context, which can be retrieved by KarmaGui::GetPlatformIO (basically GKarmaGui->PlatformIO)
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplOpenGL3_InitPlatformInterface();

		/**
		 * @brief Destroy window(s) opened in viewport(s) and clear the data
		 * 
		 * @note The job of backend here is to clear the data (that may have got stored in e.g. PlatformUserData, RendererUserData) by self before KarmaGui clears in a systematic way
		 * @todo KarmaGuiOpenGLHandler seems to be not doing the right form of cleaning of resources because Renderer_Destroy_Window is not set like that in KarmaGuiVulkanHandler. Try to find a way.
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplOpenGL3_ShutdownPlatformInterface();

		/**
		 * @brief Function called when KarmaGui layer shuts down (happens when LayerStack is destroyed). Call stack is like so: KarmaGuiLayer::OnDetach() -> KarmaGuiRenderer::OnKarmaGuiLayerDetach() -> KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_Shutdown().
		 *
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplOpenGL3_Shutdown();

		/**
		 * @brief Function called in the begining of each KarmaGui's render loop.
		 *
		 * In minor detail, the sequence is like so
		 *  1. Application::Run
		 *  2. m_KarmaGuiLayer->Begin() ->  KarmaGuiRenderer::OnKarmaGuiLayerBegin() -> KarmaGui_ImplOpenGL3_NewFrame()
		 *  3. each layer->KarmaGuiRender(deltaTime);
		 *  4. m_KarmaGuiLayer->End();
		 *
		 * @note Should be called after ImGui_ImplOpenGL3_Init() because BackendRendererUserData (io.BackendRendererUserData) must be already instantiated and initialized.
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplOpenGL3_NewFrame();

		/**
		 * @brief The routine for rendering the Karma's UI using OpenGL3 API
		 *
		 * Basically all that is drawn for Karma's UI (and inside the UI, including text, textures, and all that) is nothing but the set of triangles (KGDrawList). The relevant information is written in KGDrawData instance by the call Karma::KarmaGui::Render()
		 *
		 * @param draw_data						The data to render a KarmaGui frame
		 *
		 * @note We use ocornut's callback technique https://github.com/ocornut/imgui/issues/475#issuecomment-169953139 to render 3d scene inside KarmaGui's UI element
		 * @see KarmaGui_ImplOpenGL3_SetupRenderState()
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplOpenGL3_RenderDrawData(KGDrawData* draw_data);
		/*
		inline static KarmaGui_ImplOpenGL3_Data* KarmaGui_ImplOpenGL3_GetBackendData()
		{
			return KarmaGui::GetCurrentContext() ? (KarmaGui_ImplOpenGL3_Data*)KarmaGui::GetIO().BackendRendererUserData : NULL;
		}*/

		/**
		 * @brief Setup render state: alpha-blending (process of combining one image with a background to create the appearance of partial or full transparency) enabled, no face culling, no depth testing, scissor enabled, polygon fill, setup viewport. Bind vertex/index buffers and setup attributes for KGDrawVert.
		 *
		 * @param draw_data							The collection of variables composing the draw data to render a KarmaGui frame
		 * @param fb_width							The width of the KarmaGui's rendering scope (draw_data->DisplaySize.x * draw_data->FramebufferScale.x), for glViewport's use.
		 * @param fb_height							The height of the KarmaGui's rendering scope (draw_data->DisplaySize.y * draw_data->FramebufferScale.y), for glViewport's use.
		 * @param vertex_array_object				The name of the OpenGl vertexarray object
		 *
		 * @todo An amature work: try using math GLM's projection matrix for orthographic view
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplOpenGL3_SetupRenderState(KGDrawData* draw_data, int fb_width, int fb_height, GLuint vertex_array_object);

		/**
		 * @brief A check for success of shader (within KarmaGuiOpenGLHandler context) compilation
		 *
		 * The following log message is printed on the occurance of compilation error detected by the check
		 * @code{}
		 * [03:06:44][error] KARMA: KarmaGuiOpenGLHandler::CheckShader: failed to compile vertex shader! With GLSL:#version 410
		 * [03:06:49][error] KARMA: ERROR: 0:3: 'layout' : syntax error: syntax error
		 * @endcode
		 *
		 * @param handle						The shader object being queried
		 * @param desc							The category name of the shader object. For instance "vertex shader" or "fragment shader"
		 * 
		 * @since Karma 1.0.0
		 */
		static bool CheckShader(GLuint handle, const char* desc);

		/**
		 * @brief A check for the success of shader (within KarmaGuiOpenGLHandler context) linking
		 *
		 * The following log message is printed on the occurance of the linking error detected by the check
		 * @code{}
		 * [03:36:56][error] KARMA: KarmaGuiOpenGLHandler::CheckProgram: failed to link shader program! With GLSL:#version 410
		 * [03:36:56][error] KARMA: ERROR: One or more attached shaders not successfully compiled
		 * @endcode
		 *
		 * @param handle						The GL program (for shader object) object being queried
		 * @param desc							The category name of the program object. For instance "shader program" (only)
		 * @since Karma 1.0.0
		 */
		static bool CheckProgram(GLuint handle, const char* desc);

		/**
		 * @brief Prepares the context for OpenGl's rendering of KarmaGui's primitives and calls the function KarmaGui_ImplOpenGL3_RenderDrawData
		 *
		 * @note This function is supplied to KarmaGuiPlatformIO's Renderer_RenderWindow member and then KarmaGuiRenderer (via call in KarmaGuiRenderer::OnKarmaGuiLayerEnd()) takes care of the rest of rendering sequence. This is done for incorporating even the helper modular design.
		 *
		 * @param viewport					The only single platform window since we are not working with multi-viewport
		 * @param render_arg				Is the value passed to KarmaGui::RenderPlatformWindowsDefault() by KarmaGuiRenderer::OnKarmaGuiLayerEnd(), usually NULL
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplOpenGL3_RenderWindow(KarmaGuiViewport* viewport, void* render_arg);

		/**
		 * @brief A helper function to generate texture, in appropriate format, and add to the list openglMesaDecalDataList for use.
		 *
		 * @param fileName						The path to the file containing texture
		 * @param label							The name of the texture for identification purpose
		 *
		 * @see OpenGLImageBuffer::SetUpImageBuffer
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplOpenGL3_CreateTexture(char const* fileName, const std::string& label = "");

		// (Optional) Called by Init/NewFrame/Shutdown
		/**
		 * @brief A function to generate texture for OpenGL fonts
		 * The following steps are taken:
		 * 1. Texture atlas is built from KGFontAtlas. This is done by first loading the font atlas in the form RGBA 32-bit (with 75%  wastage). Then the
		 * 	creation of a 2D texture (GL_TEXTURE_2D), with appropriate name(s), by glGenTextures -> glBindTexture -> glTexImage2D -> target texture (or atlas)
		 * 2. Finally the target texture is stored like so; io.Fonts->SetTexID
		 *
		 * @since Karma 1.0.0
		 */
		static bool KarmaGui_ImplOpenGL3_CreateFontsTexture();

		/**
		 * @brief Clear up the resources occupied by the fonts and decals, all the textures in this context.
		 *
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplOpenGL3_DestroyFontsTexture();

		/**
		 * @brief Creates and generates shaders and buffers to be used along with the necessary uniforms
		 *
		 * @since Karma 1.0.0
		 */
		static bool KarmaGui_ImplOpenGL3_CreateDeviceObjects();

		/**
		 * @brief Clears up all the resources (shaders, buffers, and uniforms) allocated in KarmaGui_ImplOpenGL3_CreateDeviceObjects()
		 *
		 * @since Karma 1.0.0
		 */
		static void KarmaGui_ImplOpenGL3_DestroyDeviceObjects();
	};
}
