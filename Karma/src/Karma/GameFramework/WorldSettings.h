/**
 * @file WorldSettings.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class AWorldSettings.
 * @version 1.0
 * @date May 8, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Actor.h"

namespace Karma
{
	/**
	 * @brief Actor containing all script accessible world properties.
	 *
	 * @note instead of AActor inheritence, that should be AInfo
	 */
	class KARMA_API AWorldSettings : public AActor
	{
		DECLARE_KARMA_CLASS(AWorldSettings, AActor)
	};
}
