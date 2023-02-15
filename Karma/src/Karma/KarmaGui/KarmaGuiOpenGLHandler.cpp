//#include "imgui.h"
#include "KarmaGuiOpenGLHandler.h"

#include <stdio.h>

#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif
#if KR_MAC_PLATFORM
#include <TargetConditionals.h>
#endif

#include "glad/glad.h"

namespace Karma
{
	// Static Functions
	bool KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_Init(const char* glsl_version)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		KR_CORE_ASSERT(io.BackendRendererUserData == NULL, "Already initialized a renderer backend!");

		// We have already initialized glad in OpenGLContext::Init()

		// Setup backend capabilities flags
		KarmaGui_ImplOpenGL3_Data* bd = KG_NEW(KarmaGui_ImplOpenGL3_Data)();
		io.BackendRendererUserData = (void*)bd;
		io.BackendRendererName = "OpenGL_Got_Back";

		// Query for GL version (e.g. 320 for GL 3.2)
#if !defined(IMGUI_IMPL_OPENGL_ES2)
		GLint major = 0;
		GLint minor = 0;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		if (major == 0 && minor == 0)
		{
			// Query GL_VERSION in desktop GL 2.x, the string will start with "<major>.<minor>"
			const char* gl_version = (const char*)glGetString(GL_VERSION);
			sscanf(gl_version, "%d.%d", &major, &minor);
		}
		bd->GlVersion = (GLuint)(major * 100 + minor * 10);

		// Query vendor to enable glBufferSubData kludge
#ifdef KR_WINDOWS_PLATFORM
		if (const char* vendor = (const char*)glGetString(GL_VENDOR))
		{
			if (strncmp(vendor, "Intel", 5) == 0)
			{
				bd->UseBufferSubData = true;
			}
		}
#endif
#else
		bd->GlVersion = 200; // GLES 2
#endif

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
		if (bd->GlVersion >= 320)
		{
			io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing 	for large meshes.
		}
#endif

		//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side 	(optional)

		// Store GLSL version string so we can refer to it later in case we recreate shaders.
		// Note: GLSL version is NOT the same as GL version. Leave this to NULL if unsure.
		if (glsl_version == NULL)
		{
#if defined(IMGUI_IMPL_OPENGL_ES2)
			glsl_version = "#version 100";
#elif defined(IMGUI_IMPL_OPENGL_ES3)
			glsl_version = "#version 300 es";
#elif defined KR_MAC_PLATFORM
			glsl_version = "#version 150";
#else
			glsl_version = "#version 130";
#endif
		}

		KR_CORE_ASSERT((int)strlen(glsl_version) + 2 < KG_ARRAYSIZE(bd->GlslVersionString), "");

		strcpy(bd->GlslVersionString, glsl_version);
		strcat(bd->GlslVersionString, "\n");

		// Make an arbitrary GL call (we don't actually need the result)
		// IF YOU GET A CRASH HERE: it probably means the OpenGL function loader didn't do its job. Let us know!
		GLint current_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);

		// Detect extensions we support
		bd->HasClipOrigin = (bd->GlVersion >= 450);

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_EXTENSIONS
		GLint num_extensions = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
		for (GLint i = 0; i < num_extensions; i++)
		{
			const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);

			if (extension != NULL && strcmp(extension, "GL_ARB_clip_control") == 0)
			{
				bd->HasClipOrigin = true;
			}
		}
#endif

		if (io.ConfigFlags & KGGuiConfigFlags_ViewportsEnable)
		{
			KarmaGui_ImplOpenGL3_InitPlatformInterface();
		}

		return true;
	}

	void KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_Shutdown()
	{
		KarmaGui_ImplOpenGL3_Data* bd = KarmaGui_ImplOpenGL3_GetBackendData();
		KR_CORE_ASSERT(bd != NULL, "No renderer backend to shutdown, or already shutdown?");
		KarmaGuiIO& io = KarmaGui::GetIO();

		KarmaGui_ImplOpenGL3_ShutdownPlatformInterface();
		KarmaGui_ImplOpenGL3_DestroyDeviceObjects();
		io.BackendRendererName = NULL;
		io.BackendRendererUserData = NULL;
		KG_DELETE(bd);
	}

	void KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_NewFrame()
	{
		KarmaGui_ImplOpenGL3_Data* bd = KarmaGui_ImplOpenGL3_GetBackendData();
		KR_CORE_ASSERT(bd != NULL, "Did you call ImGui_ImplOpenGL3_Init()?");

		if (!bd->ShaderHandle)
		{
			KarmaGui_ImplOpenGL3_CreateDeviceObjects();
		}
	}

	void KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_SetupRenderState(KGDrawData* draw_data, int fb_width, int fb_height, GLuint vertex_array_object)
	{
		KarmaGui_ImplOpenGL3_Data* bd = KarmaGui_ImplOpenGL3_GetBackendData();

		// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glEnable(GL_SCISSOR_TEST);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
		if (bd->GlVersion >= 310)
			glDisable(GL_PRIMITIVE_RESTART);
#endif
#ifdef IMGUI_IMPL_HAS_POLYGON_MODE
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

		// Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
#if defined(GL_CLIP_ORIGIN)
		bool clip_origin_lower_left = true;
		if (bd->HasClipOrigin)
		{
			GLenum current_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&current_clip_origin);
			if (current_clip_origin == GL_UPPER_LEFT)
				clip_origin_lower_left = false;
		}
#endif

		// Setup viewport, orthographic projection matrix
		// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize 	(bottom right). DisplayPos is (0,0) for single viewport apps.
		glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
#if defined(GL_CLIP_ORIGIN)
		if (!clip_origin_lower_left) { float tmp = T; T = B; B = tmp; } // Swap top and bottom if origin is upper left
#endif
		const float ortho_projection[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
			{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
			{ 0.0f,         0.0f,        -1.0f,   0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
		};
		glUseProgram(bd->ShaderHandle);
		glUniform1i(bd->AttribLocationTex, 0);
		glUniformMatrix4fv(bd->AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
		if (bd->GlVersion >= 330)
			glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#endif

		(void)vertex_array_object;
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		glBindVertexArray(vertex_array_object);
#endif

		// Bind vertex/index buffers and setup attributes for ImDrawVert
		glBindBuffer(GL_ARRAY_BUFFER, bd->VboHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bd->ElementsHandle);
		glEnableVertexAttribArray(bd->AttribLocationVtxPos);
		glEnableVertexAttribArray(bd->AttribLocationVtxUV);
		glEnableVertexAttribArray(bd->AttribLocationVtxColor);
		glVertexAttribPointer(bd->AttribLocationVtxPos, 2, GL_FLOAT, GL_FALSE, sizeof(KGDrawVert), (GLvoid*)KG_OFFSETOF(KGDrawVert, pos));
		glVertexAttribPointer(bd->AttribLocationVtxUV, 2, GL_FLOAT, GL_FALSE, sizeof(KGDrawVert), (GLvoid*)KG_OFFSETOF(KGDrawVert, uv));
		glVertexAttribPointer(bd->AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(KGDrawVert), (GLvoid*)KG_OFFSETOF(KGDrawVert, col));
	}

	// OpenGL3 Render function.
	// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly.
	// This is in order to be able to run within an OpenGL engine that doesn't do so.
	void KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_RenderDrawData(KGDrawData* draw_data)
	{
		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
		int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);

		if (fb_width <= 0 || fb_height <= 0)
		{
			return;
		}

		KarmaGui_ImplOpenGL3_Data* bd = KarmaGui_ImplOpenGL3_GetBackendData();

		// Backup GL state
		GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
		glActiveTexture(GL_TEXTURE0);
		GLuint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&last_program);
		GLuint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&last_texture);

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
		GLuint last_sampler; if (bd->GlVersion >= 330) { glGetIntegerv(GL_SAMPLER_BINDING, (GLint*)&last_sampler); }
		else { last_sampler = 0; }
#endif
		GLuint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		// This is part of VAO on OpenGL 3.0+ and OpenGL ES 3.0+.
		GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
		ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_pos; 	last_vtx_attrib_state_pos.GetState(bd->AttribLocationVtxPos);
		ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_uv; last_vtx_attrib_state_uv.GetState(bd->AttribLocationVtxUV);
		ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_color; 	last_vtx_attrib_state_color.GetState(bd->AttribLocationVtxColor);
#endif
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		GLuint last_vertex_array_object; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&last_vertex_array_object);
#endif
#ifdef IMGUI_IMPL_HAS_POLYGON_MODE
		GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
		GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
		GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
		GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
		GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
		GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
		GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
		GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
		GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
		GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
		GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
		GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
		GLboolean last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);
		GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
		GLboolean last_enable_primitive_restart = (bd->GlVersion >= 310) ? glIsEnabled(GL_PRIMITIVE_RESTART) : GL_FALSE;
#endif

		// Setup desired GL state
		// Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among 	GL contexts)
		// The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default 	one currently bound.
		GLuint vertex_array_object = 0;
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		glGenVertexArrays(1, &vertex_array_object);
#endif
		KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);

		// Will project scissor/clipping rectangles into framebuffer space
		KGVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
		KGVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const KGDrawList* cmd_list = draw_data->CmdLists[n];

			// Upload vertex/index buffers
			// - On Intel windows drivers we got reports that regular glBufferData() led to accumulating leaks when using 	multi-viewports, so we started using orphaning + glBufferSubData(). (See 	https://github.com/ocornut/imgui/issues/4468)
			// - On NVIDIA drivers we got reports that using orphaning + glBufferSubData() led to glitches when using 	multi-viewports.
			// - OpenGL drivers are in a very sorry state in 2022, for now we are switching code path based on vendors.
			const GLsizeiptr vtx_buffer_size = (GLsizeiptr)cmd_list->VtxBuffer.Size * (int)sizeof(KGDrawVert);
			const GLsizeiptr idx_buffer_size = (GLsizeiptr)cmd_list->IdxBuffer.Size * (int)sizeof(KGDrawIdx);
			if (bd->UseBufferSubData)
			{
				if (bd->VertexBufferSize < vtx_buffer_size)
				{
					bd->VertexBufferSize = vtx_buffer_size;
					glBufferData(GL_ARRAY_BUFFER, bd->VertexBufferSize, NULL, GL_STREAM_DRAW);
				}
				if (bd->IndexBufferSize < idx_buffer_size)
				{
					bd->IndexBufferSize = idx_buffer_size;
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, bd->IndexBufferSize, NULL, GL_STREAM_DRAW);
				}
				glBufferSubData(GL_ARRAY_BUFFER, 0, vtx_buffer_size, (const GLvoid*)cmd_list->VtxBuffer.Data);
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, idx_buffer_size, (const GLvoid*)cmd_list->IdxBuffer.Data);
			}
			else
			{
				glBufferData(GL_ARRAY_BUFFER, vtx_buffer_size, (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_buffer_size, (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);
			}

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const KGDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback != NULL)
				{
					// User callback, registered via ImDrawList::AddCallback()
					// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to 	reset render state.)
					if (pcmd->UserCallback == KGDrawCallback_ResetRenderState)
						KarmaGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
					else
						pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					// Project scissor/clipping rectangles into framebuffer space
					KGVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
					KGVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
					if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
						continue;

					// Apply scissor/clipping rectangle (Y is inverted in OpenGL)
					glScissor((int)clip_min.x, (int)((float)fb_height - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y));

					// Bind texture, Draw
					glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID());
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
					if (bd->GlVersion >= 320)
						glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)), (GLint)pcmd->VtxOffset);
					else
#endif
						glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(KGDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(KGDrawIdx)));
				}
			}
		}

		// Destroy the temporary VAO
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		glDeleteVertexArrays(1, &vertex_array_object);
#endif

		// Restore modified GL state
		glUseProgram(last_program);
		glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
		if (bd->GlVersion >= 330)
			glBindSampler(0, last_sampler);
#endif
		glActiveTexture(last_active_texture);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		glBindVertexArray(last_vertex_array_object);
#endif
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
		last_vtx_attrib_state_pos.SetState(bd->AttribLocationVtxPos);
		last_vtx_attrib_state_uv.SetState(bd->AttribLocationVtxUV);
		last_vtx_attrib_state_color.SetState(bd->AttribLocationVtxColor);
#endif
		glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
		glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
		if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
		if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
		if (last_enable_stencil_test) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);
		if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
		if (bd->GlVersion >= 310) { if (last_enable_primitive_restart) glEnable(GL_PRIMITIVE_RESTART); else 	glDisable(GL_PRIMITIVE_RESTART); }
#endif

#ifdef IMGUI_IMPL_HAS_POLYGON_MODE
		glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
		glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
		glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
		(void)bd; // Not all compilation paths use this
	}

	bool KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateFontsTexture()
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		KarmaGui_ImplOpenGL3_Data* bd = KarmaGui_ImplOpenGL3_GetBackendData();

		// Build texture atlas
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but 	default font is so small) because it is more likely to be compatible with user's existing shaders. If your 	ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead 	to save on GPU memory.

		// Upload texture to graphics system
		// (Bilinear sampling is required by default. Set 'io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 	'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling)
		GLint last_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGenTextures(1, &bd->FontTexture);

		glBindTexture(GL_TEXTURE_2D, bd->FontTexture);

		for (auto& decal : bd->mesaDecalDataList)
		{
			glBindTexture(1, decal.DecalRef);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		// Store our identifier
		io.Fonts->SetTexID((KGTextureID)(intptr_t)bd->FontTexture);

		for (auto& decal : bd->mesaDecalDataList)
		{
			decal.DecalID = (KGTextureID)(intptr_t)decal.DecalRef;
		}

		// Restore state
		glBindTexture(GL_TEXTURE_2D, last_texture);

		return true;
	}

	// We gonna destroy decals texture on the ride!
	void KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_DestroyFontsTexture()
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		KarmaGui_ImplOpenGL3_Data* bd = KarmaGui_ImplOpenGL3_GetBackendData();
		if (bd->FontTexture)
		{
			glDeleteTextures(1, &bd->FontTexture);
			io.Fonts->SetTexID(0);
			bd->FontTexture = 0;
		}

		for (auto& decal : bd->mesaDecalDataList)
		{
			glDeleteTextures(1, &decal.DecalRef);
			decal.DecalID = 0;
		}
	}

	// If you get an error please report on github. You may try different GL context version or GLSL version. See GL<>GLSL 	version table at the top of this file.
	bool KarmaGuiOpenGLHandler::CheckShader(GLuint handle, const char* desc)
	{
		KarmaGui_ImplOpenGL3_Data* bd = KarmaGui_ImplOpenGL3_GetBackendData();
		GLint status = 0, log_length = 0;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
		if ((GLboolean)status == GL_FALSE)
			fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile %s! With GLSL: %s\n", desc, bd->GlslVersionString);
		if (log_length > 1)
		{
			KGVector<char> buf;
			buf.resize((int)(log_length + 1));
			glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
			fprintf(stderr, "%s\n", buf.begin());
		}
		return (GLboolean)status == GL_TRUE;
	}

	// If you get an error please report on GitHub. You may try different GL context version or GLSL version.
	bool KarmaGuiOpenGLHandler::CheckProgram(GLuint handle, const char* desc)
	{
		KarmaGui_ImplOpenGL3_Data* bd = KarmaGui_ImplOpenGL3_GetBackendData();
		GLint status = 0, log_length = 0;
		glGetProgramiv(handle, GL_LINK_STATUS, &status);
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
		if ((GLboolean)status == GL_FALSE)
			fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link %s! With GLSL %s\n", desc, bd->GlslVersionString);
		if (log_length > 1)
		{
			KGVector<char> buf;
			buf.resize((int)(log_length + 1));
			glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
			fprintf(stderr, "%s\n", buf.begin());
		}
		return (GLboolean)status == GL_TRUE;
	}

	bool KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_CreateDeviceObjects()
	{
		KarmaGui_ImplOpenGL3_Data* bd = KarmaGui_ImplOpenGL3_GetBackendData();

		// Backup GL state
		GLint last_texture, last_array_buffer;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);

#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		GLint last_vertex_array;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#endif

		// Parse GLSL version string
		int glsl_version = 130;
		sscanf(bd->GlslVersionString, "#version %d", &glsl_version);

		const GLchar* vertex_shader_glsl_120 =
			"uniform mat4 ProjMtx;\n"
			"attribute vec2 Position;\n"
			"attribute vec2 UV;\n"
			"attribute vec4 Color;\n"
			"varying vec2 Frag_UV;\n"
			"varying vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"    Frag_UV = UV;\n"
			"    Frag_Color = Color;\n"
			"    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const GLchar* vertex_shader_glsl_130 =
			"uniform mat4 ProjMtx;\n"
			"in vec2 Position;\n"
			"in vec2 UV;\n"
			"in vec4 Color;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"    Frag_UV = UV;\n"
			"    Frag_Color = Color;\n"
			"    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const GLchar* vertex_shader_glsl_300_es =
			"precision highp float;\n"
			"layout (location = 0) in vec2 Position;\n"
			"layout (location = 1) in vec2 UV;\n"
			"layout (location = 2) in vec4 Color;\n"
			"uniform mat4 ProjMtx;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"    Frag_UV = UV;\n"
			"    Frag_Color = Color;\n"
			"    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const GLchar* vertex_shader_glsl_410_core =
			"layout (location = 0) in vec2 Position;\n"
			"layout (location = 1) in vec2 UV;\n"
			"layout (location = 2) in vec4 Color;\n"
			"uniform mat4 ProjMtx;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"    Frag_UV = UV;\n"
			"    Frag_Color = Color;\n"
			"    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const GLchar* fragment_shader_glsl_120 =
			"#ifdef GL_ES\n"
			"    precision mediump float;\n"
			"#endif\n"
			"uniform sampler2D Texture;\n"
			"varying vec2 Frag_UV;\n"
			"varying vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"    gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);\n"
			"}\n";

		const GLchar* fragment_shader_glsl_130 =
			"uniform sampler2D Texture;\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
			"}\n";

		const GLchar* fragment_shader_glsl_300_es =
			"precision mediump float;\n"
			"uniform sampler2D Texture;\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"layout (location = 0) out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
			"}\n";

		const GLchar* fragment_shader_glsl_410_core =
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"uniform sampler2D Texture;\n"
			"layout (location = 0) out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
			"}\n";

		// Select shaders matching our GLSL versions
		const GLchar* vertex_shader = NULL;
		const GLchar* fragment_shader = NULL;
		if (glsl_version < 130)
		{
			vertex_shader = vertex_shader_glsl_120;
			fragment_shader = fragment_shader_glsl_120;
		}
		else if (glsl_version >= 410)
		{
			vertex_shader = vertex_shader_glsl_410_core;
			fragment_shader = fragment_shader_glsl_410_core;
		}
		else if (glsl_version == 300)
		{
			vertex_shader = vertex_shader_glsl_300_es;
			fragment_shader = fragment_shader_glsl_300_es;
		}
		else
		{
			vertex_shader = vertex_shader_glsl_130;
			fragment_shader = fragment_shader_glsl_130;
		}

		// Create shaders
		const GLchar* vertex_shader_with_version[2] = { bd->GlslVersionString, vertex_shader };
		GLuint vert_handle = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert_handle, 2, vertex_shader_with_version, NULL);
		glCompileShader(vert_handle);
		CheckShader(vert_handle, "vertex shader");

		const GLchar* fragment_shader_with_version[2] = { bd->GlslVersionString, fragment_shader };
		GLuint frag_handle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag_handle, 2, fragment_shader_with_version, NULL);
		glCompileShader(frag_handle);
		CheckShader(frag_handle, "fragment shader");

		// Link
		bd->ShaderHandle = glCreateProgram();
		glAttachShader(bd->ShaderHandle, vert_handle);
		glAttachShader(bd->ShaderHandle, frag_handle);
		glLinkProgram(bd->ShaderHandle);
		CheckProgram(bd->ShaderHandle, "shader program");

		glDetachShader(bd->ShaderHandle, vert_handle);
		glDetachShader(bd->ShaderHandle, frag_handle);
		glDeleteShader(vert_handle);
		glDeleteShader(frag_handle);

		bd->AttribLocationTex = glGetUniformLocation(bd->ShaderHandle, "Texture");
		bd->AttribLocationProjMtx = glGetUniformLocation(bd->ShaderHandle, "ProjMtx");
		bd->AttribLocationVtxPos = (GLuint)glGetAttribLocation(bd->ShaderHandle, "Position");
		bd->AttribLocationVtxUV = (GLuint)glGetAttribLocation(bd->ShaderHandle, "UV");
		bd->AttribLocationVtxColor = (GLuint)glGetAttribLocation(bd->ShaderHandle, "Color");

		// Create buffers
		glGenBuffers(1, &bd->VboHandle);
		glGenBuffers(1, &bd->ElementsHandle);

		KarmaGui_ImplOpenGL3_CreateFontsTexture();

		// Restore modified GL state
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		glBindVertexArray(last_vertex_array);
#endif

		return true;
	}

	void  KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_DestroyDeviceObjects()
	{
		KarmaGui_ImplOpenGL3_Data* bd = KarmaGui_ImplOpenGL3_GetBackendData();
		if (bd->VboHandle) { glDeleteBuffers(1, &bd->VboHandle); bd->VboHandle = 0; }
		if (bd->ElementsHandle) { glDeleteBuffers(1, &bd->ElementsHandle); bd->ElementsHandle = 0; }
		if (bd->ShaderHandle) { glDeleteProgram(bd->ShaderHandle); bd->ShaderHandle = 0; }
		KarmaGui_ImplOpenGL3_DestroyFontsTexture();
	}

	//--------------------------------------------------------------------------------------------------------
	// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
	// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports 	simultaneously.
	// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this 	section first..
	//--------------------------------------------------------------------------------------------------------

	void KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_RenderWindow(KarmaGuiViewport* viewport, void*)
	{
		if (!(viewport->Flags & KGGuiViewportFlags_NoRendererClear))
		{
			KGVec4 clear_color = KGVec4(0.0f, 0.0f, 0.0f, 1.0f);
			glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);
		}
		KarmaGui_ImplOpenGL3_RenderDrawData(viewport->DrawData);
	}

	void KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_InitPlatformInterface()
	{
		KarmaGuiPlatformIO& platform_io = KarmaGui::GetPlatformIO();
		platform_io.Renderer_RenderWindow = KarmaGui_ImplOpenGL3_RenderWindow;
	}

	void KarmaGuiOpenGLHandler::KarmaGui_ImplOpenGL3_ShutdownPlatformInterface()
	{
		KarmaGui::DestroyPlatformWindows();
	}
}