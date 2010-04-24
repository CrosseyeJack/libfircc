#ifndef _PLUGIN_FUNCTIONS_H_
#define _PLUGIN_FUNCTIONS_H_

#include <eventdispatcher.h>
#include <networkmanagerevents.h>

namespace anp
{
namespace firc
{
	// Abbreviations/prefixes:
	// pf - plugin function, implemented in plugin but callable from core
	// cf - core function, implemented in core but callable from plugin
	
	class INetworkManagerFrontend;
	class MsgPrefix;


	// Typedefs
	// uint32 pluginInit(void *fircCore)
	typedef uint32 (*PF_pluginInit)(
		anp::EventDispatcher<
			events::ISubscriber<events::NewSession>,
			events::NewSession
		> &newSessionDispatcher,
		void *appContext
	);
	
	// void pluginDeinit()
	typedef void (*PF_pluginDeinit)(uint32);
	
	// void irc_onJoin(void *network, const int8 *channel, const int8 *user)
	typedef void (*PF_irc_onJoin)(	INetworkManagerFrontend &,
									const MsgPrefix &,
									const int8 *);

	typedef void (*PF_irc_onPart)(	INetworkManagerFrontend &,
									const MsgPrefix &,
									const int8 *,
									const int8 *);
	
	typedef void (*PF_irc_onPrivMsg)(INetworkManagerFrontend &,
									const MsgPrefix &,
									const int8 *,
									const int8 *);

	typedef void (*PF_irc_onTopic)(INetworkManagerFrontend &,
									const MsgPrefix &,
									const int8 *,
									const int8 *);
} // namespace firc
} // namespace anp

#endif // _PLUGIN_FUNCTIONS_H_
