/**
 * @file imgui_impl_glfw.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains GLFW renderer backend
 * @version 1.0
 * @date February 7, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */

// dear imgui: Platform Backend for GLFW
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
// (Info: GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (Requires: GLFW 3.1+. Prefer GLFW 3.3+ for full feature support.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass KarmaGuiKey values to all key functions e.g. KarmaGui::IsKeyPressed(KarmaGuiKey_Space). [Legacy GLFW_KEY_* values will also be supported unless IMGUI_DISABLE_OBSOLETE_KEYIO is set]
//  [X] Platform: Gamepad support. Enable with 'io.ConfigFlags |= KarmaGuiConfigFlags_NavEnableGamepad'.
//  [x] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= KarmaGuiConfigFlags_NoMouseCursorChange' (note: the resizing cursors requires GLFW 3.4+).
//  [X] Platform: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |= KarmaGuiConfigFlags_ViewportsEnable'.

// Issues:
//  [ ] Platform: Multi-viewport support: ParentViewportID not honored, and so io.ConfigViewportsNoDefaultParent has no effect (minor).

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// If you are new to Dear KarmaGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#pragma once
#include "KarmaGui.h"

struct GLFWwindow;
struct GLFWmonitor;

namespace Karma
{
	bool     KarmaGui_ImplGlfw_InitForOpenGL(GLFWwindow* window, bool install_callbacks);
	bool     KarmaGui_ImplGlfw_InitForVulkan(GLFWwindow* window, bool install_callbacks);
	bool     KarmaGui_ImplGlfw_InitForOther(GLFWwindow* window, bool install_callbacks);
	void     KarmaGui_ImplGlfw_Shutdown();
	void     KarmaGui_ImplGlfw_NewFrame();

	// GLFW callbacks (installer)
	// - When calling Init with 'install_callbacks=true': KarmaGui_ImplGlfw_InstallCallbacks() is called. GLFW callbacks will be installed for you. They will chain-call user's previously installed callbacks, if any.
	// - When calling Init with 'install_callbacks=false': GLFW callbacks won't be installed. You will need to call individual function yourself from your own GLFW callbacks.
	void     KarmaGui_ImplGlfw_InstallCallbacks(GLFWwindow* window);
	void     KarmaGui_ImplGlfw_RestoreCallbacks(GLFWwindow* window);

	// GLFW callbacks (individual callbacks to call if you didn't install callbacks)
	void     KarmaGui_ImplGlfw_WindowFocusCallback(GLFWwindow* window, int focused);        // Since 1.84
	void     KarmaGui_ImplGlfw_CursorEnterCallback(GLFWwindow* window, int entered);        // Since 1.84
	void     KarmaGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double x, double y);   // Since 1.87
	void     KarmaGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void     KarmaGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void     KarmaGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void     KarmaGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c);
	void     KarmaGui_ImplGlfw_MonitorCallback(GLFWmonitor* monitor, int event);
}
