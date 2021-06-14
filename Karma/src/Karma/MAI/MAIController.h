#pragma once

#include "Karma/Core.h"

namespace Karma
{
	class KARMA_API MAIController
	{
	public:
		MAIController();
		~MAIController();

		void Tick(float deltaTime);
	};
}