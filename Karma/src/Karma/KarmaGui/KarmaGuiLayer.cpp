#include "KarmaGuiLayer.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Karma/Application.h"
#include "glm/glm.hpp"
#include "Renderer/Renderer.h"
#include "KarmaGuiRenderer.h"

namespace Karma
{
	KarmaGuiLayer::KarmaGuiLayer(Window* relevantWindow)
		: Layer("ImGuiLayer"), m_AssociatedWindow(relevantWindow)
	{
		// Is this really required?
		if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			// Curate the data associated with Window context and VulkanAPI
			int width, height;
			GLFWwindow* window = static_cast<GLFWwindow*>(m_AssociatedWindow->GetNativeWindow());

			glfwGetFramebufferSize(window, &width, &height);
		}
	}

	KarmaGuiLayer::~KarmaGuiLayer()
	{
	}

	void KarmaGuiLayer::OnAttach()
	{
		KarmaGui::CreateContext();

		KarmaGuiIO& io = KarmaGui::GetIO();
		(void)io;
		io.ConfigFlags |= KGGuiConfigFlags_NavEnableKeyboard;	// Enable Keyboard Controls
		io.ConfigFlags |= KGGuiConfigFlags_DockingEnable;		// Enable Docking
		io.ConfigFlags |= KGGuiConfigFlags_ViewportsEnable;		// Enable Multi-Viewport / Platform Windows

		// Setup KarmaGui color style
		KarmaGui::StyleColorsKarma();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		KarmaGuiStyle& style = KarmaGui::GetStyle();
		if (io.ConfigFlags & KGGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[KGGuiCol_WindowBg].w = 1.0f;
		}

		// Setting Dear ImGui ini file
		io.IniFilename = "../Resources/Misc/KarmaGuiEditor.ini";//"yeehaw!";

		GLFWwindow* window = static_cast<GLFWwindow*>(m_AssociatedWindow->GetNativeWindow());

		// Setup Platform/Renderer bindings. Also assign relevant backends
		KarmaGuiRenderer::SetUpKarmaGuiRenderer(window);
	}

	void KarmaGuiLayer::OnDetach()
	{
		KarmaGuiRenderer::OnKarmaGuiLayerDetach();
		KR_CORE_INFO("Shutting down KarmaGuiLayer");
	}

	// The KarmaGuiLayer sequence begins
	void KarmaGuiLayer::Begin()
	{
		KarmaGuiRenderer::OnKarmaGuiLayerBegin();
		KarmaGui::NewFrame();
	}

	void KarmaGuiLayer::KarmaGuiRender(float deltaTime)
	{
	}

	void KarmaGuiLayer::End()
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = KGVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		KarmaGui::Render();

		KarmaGuiRenderer::OnKarmaGuiLayerEnd();
	}
	// The KarmaGuiLayer sequence ends

	void KarmaGuiLayer::OnUpdate(float deltaTime)
	{
		// Nothing to do
	}

	void KarmaGuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressedEvent>(KR_BIND_EVENT_FN(KarmaGuiLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(KR_BIND_EVENT_FN(KarmaGuiLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseMovedEvent>(KR_BIND_EVENT_FN(KarmaGuiLayer::OnMouseMovedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(KR_BIND_EVENT_FN(KarmaGuiLayer::OnMouseScrollEvent));
		dispatcher.Dispatch<KeyPressedEvent>(KR_BIND_EVENT_FN(KarmaGuiLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(KR_BIND_EVENT_FN(KarmaGuiLayer::OnKeyReleasedEvent));
		//dispatcher.Dispatch<KeyTypedEvent>(KR_BIND_EVENT_FN(ImGuiLayer::OnKeyTypedEvent));
		dispatcher.Dispatch<WindowResizeEvent>(KR_BIND_EVENT_FN(KarmaGuiLayer::OnWindowResizeEvent));
	}

	bool KarmaGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = true;

		return false;
	}

	bool KarmaGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		io.MouseDown[e.GetMouseButton()] = false;

		return false;
	}

	bool KarmaGuiLayer::OnMouseMovedEvent(MouseMovedEvent& e)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		io.MousePos = KGVec2(e.GetX(), e.GetY());

		return false;
	}

	bool KarmaGuiLayer::OnMouseScrollEvent(MouseScrolledEvent& e)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		io.MouseWheelH += e.GetXOffset();
		io.MouseWheel += e.GetYOffset();

		return false;
	}

	bool KarmaGuiLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = true;

		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

		return false;
	}

	bool KarmaGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& e)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		io.KeysDown[e.GetKeyCode()] = false;

		return false;
	}

	bool KarmaGuiLayer::OnKeyTypedEvent(KeyTypedEvent& e)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		int keycode = e.GetKeyCode();
		if (keycode > 0 && keycode < 0x10000)
		{
			io.AddInputCharacter((unsigned short)keycode);
		}

		return false;
	}

	bool KarmaGuiLayer::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		KarmaGuiIO& io = KarmaGui::GetIO();
		io.DisplaySize = KGVec2(float(e.GetWidth()), float(e.GetHeight()));
		io.DisplayFramebufferScale = KGVec2(1.0f, 1.0f);

		return false;
	}
}
