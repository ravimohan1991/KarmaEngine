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
	/**
	 * @brief Enumeration of supported RHI interface types.
	 */
	enum class ERHIInterfaceType
	{
		/**
		 * @brief Hidden or null RHI interface type.
		 */
		Hidden,
		Null,
		D3D11,
		D3D12,

		/**
		 * @brief Vulkan RHI interface type.
		 */
		Vulkan,
		Metal,
		Agx,
		OpenGL,
	};

	/**
	 * @brief Initializes the RHI.
	 * 
	 * @see Application::Application()
	 * @since Karma 1.0.0
	 */
	extern KARMA_API void RHIInit();

	/**
	 * @brief Shuts down the RHI.
	 * 
	 * @see Application::~Application()
	 * @since Karma 1.0.0
	 */
	extern KARMA_API void RHIExit();
}