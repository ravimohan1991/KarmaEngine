#pragma once

#include "Karma/Window.h"

struct GLFWwindow;

namespace Karma
{
	class GraphicsContext;
	
	class MacWindow : public Window
	{
	public:
		MacWindow(const WindowProps& props);
		virtual ~MacWindow();

		void OnUpdate() override;
        
        virtual bool OnResize(WindowResizeEvent& event) override { return true; }

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		inline void SetEventCallback(const EventCallbackFn& callback) override
		{
			m_Data.EventCallback = callback;
		}
        inline virtual void* GetNativeWindow() const override { return m_Window; }
        
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

	private:
		virtual void Init(const WindowProps& props);
		
		void SetGLFWCallbacks(GLFWwindow* glfwWindow);
		virtual void ShutDown();

		GLFWwindow* m_Window;
		GraphicsContext* m_Context;

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
