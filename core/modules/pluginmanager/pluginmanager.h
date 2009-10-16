#ifndef _PLUGINMANAGER_H_
#define _PLUGINMANAGER_H_

#include <basedefs.h>
#include <plugin_functions.h>
#include <anp_jobqueue.h>

#include <vector> // johnny bigert this!
#include <map>
#include "plugin_jobs.h" // remove this dependency by moving the stuff in this header to another

namespace anp
{
namespace firc
{
	class Plugin;

	class PluginManager
	{
	public:
		enum CallbackType
		{
			IRC_JOIN,
			IRC_PRIVMSG,
			
			IRC_MAX_CALLBACKS
		};

		PluginManager();

		void setFircCore(void *fircCore); // Extend
	
		Result loadPlugins(uint32 count, const int8 *fileNames[]);
		Result loadPlugin(const int8 *fileName);
		Result unloadAllPlugins();
		Result unloadPlugin(uint32 index, uint32 reason);
		Result unloadPluginReally(uint32 index, uint32 reason);
		Result getPluginCount(uint32 *count) const;
		Result getPluginInfo(uint32 index,
							int8 *name, uint32 nameLength);
		
		void performJob(PluginJob *job, CallbackType type);
							
		Result addCallbackOnPrivMsg(PF_irc_onPrivMsg func);
	private:
		void addPluginToUnloadList(Plugin *const plugin);
		void unloadScheduledPlugins();
		
		struct CallbackEntry
		{
			void *func;
			Plugin *plugin;
		};
	
		void							*m_fircCore; // Extend
		uint32							m_pluginCount;
		std::vector<Plugin *>			m_plugins;
		std::vector<Plugin *>			m_pluginsToBeUnloaded;
		
		std::vector<CallbackEntry> m_callbacks[IRC_MAX_CALLBACKS];
										
		anp::threading::JobQueue		m_jobQueue;
	};

} // namespace firc
} // namespace anp

#endif // _PLUGINMANAGER_H_
