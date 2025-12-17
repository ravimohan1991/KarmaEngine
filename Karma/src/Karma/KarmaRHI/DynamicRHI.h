/**
 * @file DynamicRHI.h
 * @brief
 * @author Tim Sweeney
 * @date December 15, 2025
 * 
 * @copyright Epic Games, Inc. All Rights Reserved.
 */

#pragma once

#include "KarmaRHI.h"

namespace Karma
{
	/**
	 * @brief Abstract base class for Dynamic Rendering Hardware Interface (RHI).
	 * 
	 * Provides an interface for dynamic rendering operations across different graphics APIs.
	 * 
	 * @since Unreal Engine 6.0.0
	 */
	class KARMA_API FDynamicRHI
	{
	public:

		static FDynamicRHI* CreateRHI();

		/**
		 * @brief Virtual destructor for FDynamicRHI.
		 */
		virtual ~FDynamicRHI() = default;

		/**
		 * @brief Initializes the RHI.
		 * 
		 * Sets up necessary resources and states for rendering.
		 * 
		 * @return true if initialization was successful, false otherwise.
		 */
		virtual bool Init() = 0;
		/**
		 * @brief Shuts down the RHI.
		 * 
		 * Cleans up resources and states used by the RHI.
		 */
		virtual void Shutdown() = 0;

		/**
		 * @brief Presents the rendered frame to the display.
		 */
		virtual void Present() = 0;
		// Additional methods for rendering operations can be added here

		virtual Karma::ERHIInterfaceType GetInterfaceType() const { return Karma::ERHIInterfaceType::Hidden; }
	};

	/** A global pointer to the dynamically bound RHI implementation. */
	extern KARMA_API FDynamicRHI* GDynamicRHI;

	/**
	 * @brief The current RHI interface type in use.
	 * 
	 * @remark This variable indicates which graphics API is currently active. Later 
	 * we can tie this to a configuration system (for instance loading configurable setting from file).
	 * 
	 * @since Karma 1.0.0
	 */
	extern KARMA_API ERHIInterfaceType GRHIInterfaceType;

	/**
	 * @brief Each platform that utilizes dynamic RHIs should implement this function
	 * 
	 * Called to create the instance of the dynamic RHI.
	 * 
	 * @since Karma 1.0.0
	 */
	extern FDynamicRHI* PlatformCreateDynamicRHI();
}