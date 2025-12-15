/**
 * @file KarmaRHI.h
 * @brief
 * @author Ravi Mohan (the_cowboy)
 * @date December 15, 2025
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

namespace Karma
{
	enum class ERHIInterfaceType
	{
		Hidden,
		Null,
		D3D11,
		D3D12,
		Vulkan,
		Metal,
		Agx,
		OpenGL,
	};

	/** Initializes the RHI. */
	extern KARMA_API void RHIInit();

	/** Shuts down the RHI. */
	extern KARMA_API void RHIExit();
}