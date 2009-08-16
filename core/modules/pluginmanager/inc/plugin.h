#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <basedefs.h>
#include "plugin_functions.h"

#include <string>

namespace firc
{
	class Plugin
	{
	public:
		Plugin();
		
		Result loadFromFile(void *fircCore, const int8 *fileName,
							PF_irc_onJoin *ioj,
							PF_irc_onPrivMsg *iopm);
		Result unload(uint32 reason);
		
		Result getName(const std::string **name);
		
		// Event handler get functions
		Result getEventHandler(PF_irc_onJoin *dest) const;
		Result getEventHandler(PF_irc_onPrivMsg *dest) const;
		
		// Execution, status, reloading
		void increaseExecutionCount();
		void decreaseExecutionCount();
	private:
		void				*m_handle;
		
		PF_pluginDeinit		m_pf_pluginDeinit;
		
		// Pure irc functions
		PF_irc_onJoin		m_pf_irc_onJoin;
		PF_irc_onPrivMsg	m_pf_irc_onPrivMsg;
		
		// Info
		std::string			m_name;
		
		// Execution
		uint32				m_executionCount;
	};
}

#endif // _PLUGIN_H_
