#include <basedefs.h>
#include <list>
#include <string>
#include <networkcache.h>
#include <channelcache.h>
#include <userinfo.h>
#include <stdexcept>

namespace anp
{
namespace firc
{
	
	// pImpl object
	class NetworkCacheImpl
	{
	public:
		const ChannelCache *channel(const std::string &name) const;
		ChannelCache *channel(const std::string &name);
		const ChannelCache *getChannelCopy(
										const std::string &name) const;
		
		UserInfo *userByName(const std::string &name);
	private:
		uint32 getTableIndex(const std::string &name) const;
		enum CONSTANTS
		{
			HASHTABLE_SIZE=40
		};
		std::list<ChannelCache *> m_channelHashTable[HASHTABLE_SIZE];
		std::list<UserInfo *> m_userHashTable[HASHTABLE_SIZE];
	};
	
	const ChannelCache *NetworkCacheImpl::channel(
										const std::string &name) const
	{
		uint32 index = getTableIndex(name);
		const std::list<ChannelCache *> &list =
											m_channelHashTable[index];
		
		std::list<ChannelCache *>::const_iterator i;
		for ( i=list.begin(); i != list.end(); i++ )
		{
			if ( (*i)->name() == name )
			{
				return (*i);
			}
		}
		
		throw std::runtime_error("Unable to find channel.");
		return NULL;
	}

	ChannelCache *NetworkCacheImpl::channel(
										const std::string &name)
	{
		uint32 index = getTableIndex(name);
		std::list<ChannelCache *> &list = m_channelHashTable[index];
		
		std::list<ChannelCache *>::iterator i;
		for ( i=list.begin(); i != list.end(); i++ )
		{
			if ( (*i)->name() == name )
			{
				return (*i);
			}
		}
		
		throw std::runtime_error("Unable to find channel.");
		return NULL;
	}
	
	
	const ChannelCache *NetworkCacheImpl::getChannelCopy(
										const std::string &name) const
	{
		const ChannelCache *const srcChannel = this->channel(name);
		return new ChannelCache(*srcChannel);
	}

	UserInfo *NetworkCacheImpl::userByName(const std::string &name) {
		// ..
	}
	
	uint32 NetworkCacheImpl::getTableIndex(
										const std::string &name) const
	{
		uint32 length = name.length();
		uint32 index = 0;
		for ( uint32 i=0; i<length; ++i )
		{
			index = name[i]*2 - name[i];
		}
		index = index % HASHTABLE_SIZE;
		return index;
	}
	
	// Wrapper functions
	
	const ChannelCache *NetworkCache::getChannelCopy(
										const std::string &name) const
	{
		return m_impl->getChannelCopy(name);
	}
	
	void NetworkCache::addUserToChannel(const std::string &name,
							  const std::string &user,
							  const std::string &host,
							  const std::string &channelName)
	{
		ChannelCache *channel = m_impl->channel(channelName);
		const UserInfo *userInfo = NULL;
		try
		{
			userInfo = m_impl->userByName(name);
		} catch ( std::runtime_error &e )
		{
			userInfo = new UserInfo(name, user, host);
		}
		channel->addUser(userInfo);
		userInfo->addChannel(channel);
	}
}
}
