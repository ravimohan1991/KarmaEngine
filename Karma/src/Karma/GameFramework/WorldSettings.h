#pragma once

#include "krpch.h"

#include "Actor.h"

namespace Karma
{
	/**
	 * Actor containing all script accessible world properties.
	 * 
	 * @note instead of AActor inheritence, that should be AInfo
	 */
	class KARMA_API AWorldSettings : public AActor
	{
		DECLARE_KARMA_CLASS(AWorldSettings, AActor)
	};
}