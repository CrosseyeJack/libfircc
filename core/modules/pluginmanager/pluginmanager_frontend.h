#ifndef _PLUGINMANAGER_FRONTEND_H_
#define _PLUGINMANAGER_FRONTEND_H_

#include <basedefs.h>

namespace anp
{
namespace firc
{
	class Plugin;
	
	class IPluginManagerFrontend
	{
	public:
		IPluginManager() { };
		virtual ~IPluginManager() { };

		virtual void loadPlugin(const int8 *fileName) = 0;
		virtual void unloadAllPlugins() = 0;
		virtual void unloadPlugin(uint32 index, uint32 reason) = 0;
		virtual void unloadPluginReally(uint32 index, uint32 reason) = 0;
		virtual uint32 getPluginCount() const = 0;
		virtual void getPluginInfo(uint32 index,
							int8 *name, uint32 nameLength) = 0;
		
		virtual void performJob(PluginJob *job, CallbackType type) = 0;
							
		virtual void addCallbackOnPrivMsg(PF_irc_onPrivMsg func) = 0;
	};
}
}

#endif // _PLUGINMANAGER_FRONTEND_H_
