#include "../inc/plugin.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <anp_dynamic_library.h>

namespace anp
{
namespace firc
{
	/**
	 * @brief
	 * Creates a plugin by loading it from file.
	 * 
	 * @param fileName
	 * NULL terminated string representing the fileName
	 */
	Plugin::Plugin(
		const int8 *fileName,
		anp::EventDispatcher<
			events::ISubscriber<events::NewSession>,
			events::NewSession
		> &newSessionDispatcher,
		void *appContext
	):
		m_lib(fileName),
		m_name("n/a"),
		m_pf_pluginDeinit(NULL),
		m_unloadReason(0)
	{		
		std::cout << "Plugin::loadFromFile: " << fileName << std::endl;
		
		PF_pluginInit pf_pluginInit	= (PF_pluginInit)m_lib.getSymbol(
														"pluginInit");
		m_pf_pluginDeinit			= (PF_pluginDeinit)m_lib.getSymbol(
														"pluginDeinit");
		
		// DynamicLibrary::getSymbol should throw an exception if
		// any of the functoins cannot be found, so this should never
		// happen.
		assert(NULL != pf_pluginInit && NULL != m_pf_pluginDeinit);
		
		// Call init function
		m_name = fileName;
		uint32 res = pf_pluginInit(
			newSessionDispatcher,
			appContext
		);
		if ( res == 0 )
		{
			std::cout << "pluginInit returned 0" << std::endl;
			throw std::runtime_error("pluginInit returned 0");
		}
	}
	
	Plugin::~Plugin()
	{
		m_pf_pluginDeinit(m_unloadReason);
	}
	
	void Plugin::setUnloadReason(uint32 reason)
	{
		m_unloadReason = reason;
	}
	
	void Plugin::setUnloading(bool32 unloading)
	{
		m_unloading = unloading;
	}
	
	bool32 Plugin::isUnloading() const
	{
		return m_unloading;
	}
	
	const std::string &Plugin::getName() const
	{
		// Trust that the caller respects the constness
		return m_name;
	}
	
	/////////////////////////
	// Get plugin handlers
	/////////////////////////

	Result Plugin::getEventHandler(PF_irc_onJoin *dest) const
	{
		Result res = RES_INVALID_PARAMETER;
		
		if ( NULL != dest )
		{
			if ( NULL != m_pf_irc_onJoin )
			{
				*dest = m_pf_irc_onJoin;
				res = RES_OK;
			} else
			{
				res = RES_NOTFOUND;
			}
		}
		
		return res;
	}
	
	Result Plugin::getEventHandler(PF_irc_onPrivMsg *dest) const
	{
		Result res = RES_INVALID_PARAMETER;
		
		if ( NULL != dest )
		{
			if ( NULL != m_pf_irc_onPrivMsg )
			{
				*dest = m_pf_irc_onPrivMsg;
				res = RES_OK;
			} else
			{
				res = RES_NOTFOUND;
			}
		}
		
		return res;
	}
	
	//////////////////////////////////
	// Execution, status, reloading
	//////////////////////////////////

	void Plugin::increaseExecutionCount()
	{
		++m_executionCount;
	}
	
	void Plugin::decreaseExecutionCount()
	{
		--m_executionCount;
		
		/// @todo Add assert and/or log if m_executionCount < 0
	}
} // namespace firc
} // namespace anp
