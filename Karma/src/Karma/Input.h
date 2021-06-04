#pragma once

#include "Karma/Core.h"
#include "Karma/Log.h"
#include <utility>

namespace Karma
{
	class KARMA_API Input
	{
	public:
		inline static bool IsKeyPressed(int keycode)
		{
			return s_Instance->IsKeyPressedImpl(keycode);
		}
		inline static bool IsMouseButtonPressed(int button)
		{
			return s_Instance->IsMouseButtonPressedImpl(button);
		}
		inline static bool IsMouseButtonReleased(int button)
		{
			return s_Instance->IsMouseButtonReleasedImpl(button);
		}
		inline static std::pair<float, float>GetMousePosition()
		{
			return s_Instance->GetMousePositionImpl();
		}
		inline static float GetMouseX()
		{
			return s_Instance->GetMouseXImpl();
		}
		inline static float GetMouseY()
		{
			return s_Instance->GetMouseYImpl();
		}

		inline static void DeleteInstance()
		{
			if (s_Instance)
			{
				delete s_Instance;
				KR_CORE_INFO("Deleted input singleton instance!");
			}
		}

	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual bool IsMouseButtonReleasedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

	private:
		static Input* s_Instance;
	};
}