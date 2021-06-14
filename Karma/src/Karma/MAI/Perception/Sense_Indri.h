#pragma once

/*
Ripped from https://github.com/EpicGames/UnrealEngine/blob/4.27/Engine/Source/Runtime/AIModule/Classes/Perception/AISense.h
*/

#include "Karma/Core.h"

namespace Karma
{
	class KARMA_API Indri
	{
	public:
		Indri();
		~Indri();

	protected:
		/** Age past which stimulus of this sense are "forgotten".*/
		float m_DefaultExpirationAge;


	};
}