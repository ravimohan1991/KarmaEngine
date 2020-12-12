#pragma once

#include "Karma/Window.h"

struct GLFWwindow;

namespace Karma
{
	class LinuxWindow : public Window
	{
	public:
		LinuxWindow(const WindowProps& props);
		virtual ~LinuxWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		inline void SetEventCallback(const EventCallbackFn& callback) override
		{
			m_Data.EventCallback = callback;
		}
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

	private:
		virtual void Init(const WindowProps& props);
		
		void SetGLFWCallbacks(GLFWwindow* glfwWindow);
		virtual void ShutDown();

		GLFWwindow* m_Window;
		
		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}