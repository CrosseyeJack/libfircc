#include <basedefs.h>
#include <vector>
#include <algorithm>
#include <string>
#include <networkcache.h>
#include <channelcache.h>
#include <userinfo.h>
#include "../inc/utilities.h"
#include <stdexcept>

namespace anp
{
namespace firc
{

	struct ChannelUserRelation
	{
		ChannelUserRelation(const std::string &channel,
									const std::string &user, uint32 modes):
		m_channel(channel),
		m_user(user),
		m_modes(modes)
		{
		}
		uint32 m_modes;
		std::string m_channel;
		std::string m_user;
	};

	static int channeluserrelation_compareC(const ChannelUserRelation &r1,
									const ChannelUserRelation &r2)
	{
		return r1.m_channel.compare(r2.m_channel);
	}

	// pImpl object
	class NetworkCacheImpl
	{
	public:
		const ChannelCache *channel(const std::string &name) const;
		ChannelCache *channel(const std::string &name);
		const ChannelCache *getChannelCopy(
										const std::string &name) const;
		
		UserInfo *userByName(const std::string &name);
		std::vector<ChannelUserRelation> m_cuRelations;
	private:
		uint32 getChannelIndex(const std::string &name) const;
		uint32 getUserIndex(const std::string &name) const;
		std::vector<ChannelCache *> m_channels;
		std::vector<UserInfo *> m_users;
	};

	const ChannelCache *NetworkCacheImpl::channel(
										const std::string &name) const
	{
		uint32 index = getChannelIndex(name);
		
		return m_channels[index];
	}

	ChannelCache *NetworkCacheImpl::channel(
										const std::string &name)
	{
		uint32 index = getChannelIndex(name);
		
		return m_channels[index];
	}
	
	
	const ChannelCache *NetworkCacheImpl::getChannelCopy(
										const std::string &name) const
	{
		const ChannelCache *const srcChannel = this->channel(name);
		return new ChannelCache(*srcChannel);
	}

	UserInfo *NetworkCacheImpl::userByName(const std::string &name) {
		UserInfo temp(name, "", "");
		std::pair<
			std::vector<UserInfo *>::iterator,
			std::vector<UserInfo *>::iterator
		> res = std::equal_range(m_users.begin(), m_users.end(),
							&temp, userinfo_compare);
		if ( res.first != m_users.end() )
		{
			return *res.first;
		} else
		{
			throw std::runtime_error("Unable to find user.");
			return NULL;
		}
	}
	
	uint32 NetworkCacheImpl::getChannelIndex(
										const std::string &name) const
	{
		// Perform a binary search
		uint32 upper = m_channels.size();
		uint32 lower = 0;
		uint32 pivot = 0;
		uint32 diff = 0;
		while ( true )
		{
			pivot = (upper-lower)/2;
			diff=name.compare(m_channels[pivot]->name());
			if ( diff < 0 )
			{
				upper = pivot;
			} else if ( diff > 0 )
			{
				lower = pivot;
			} else // diff = 0
			{
				// Match
				return pivot;
			}
			// TODO: If (upper-lower) is small, what happens?
			// Will all positions be examined?
			// Do we need to roundup pivot calculation?
		}
		throw std::runtime_error("Unable to find channel.");
	}

	uint32 NetworkCacheImpl::getUserIndex(
										const std::string &name) const
	{
		// Perform a binary search
		uint32 upper = m_users.size();
		uint32 lower = 0;
		uint32 pivot = 0;
		uint32 diff = 0;
		while ( true )
		{
			pivot = (upper-lower)/2;
			diff=name.compare(m_users[pivot]->name());
			if ( diff < 0 )
			{
				upper = pivot;
			} else if ( diff > 0 )
			{
				lower = pivot;
			} else // diff = 0
			{
				// Match
				return pivot;
			}
			// TODO: If (upper-lower) is small, what happens?
			// Will all positions be examined?
			// Do we need to roundup pivot calculation?
		}
		throw std::runtime_error("Unable to find channel.");
	}

	// Wrapper functions
	NetworkCache::NetworkCache():
	m_impl(new NetworkCacheImpl)
	{
	}

	NetworkCache::~NetworkCache()
	{
		delete m_impl;
	}
	
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
		UserInfo *userInfo = NULL;
		try
		{
			userInfo = m_impl->userByName(name);
		} catch ( std::runtime_error &e )
		{
			userInfo = new UserInfo(name, user, host);
		}
		ChannelUserRelation newRelation(channelName, name, 0);
		std::vector<ChannelUserRelation> &table = m_impl->m_cuRelations;
		table.insert(
			std::lower_bound(
				table.begin(),
				table.end(),
				newRelation,
				channeluserrelation_compareC
			),
			newRelation
		);
	}
	
	void NetworkCache::removeUserFromChannel(const std::string &name,
								const std::string &channelName)
	{
		ChannelUserRelation tempRel(channelName, name, 0);
		std::vector<ChannelUserRelation> &table = m_impl->m_cuRelations;
		std::pair<
			std::vector<ChannelUserRelation>::iterator,
			std::vector<ChannelUserRelation>::iterator
		> range = std::equal_range(table.begin(), table.end(), tempRel,
									channeluserrelation_compareC);

		if ( range.first != table.end() )
		{
			while ( range.first != range.second )
			{
				if ( (*range.first).m_user == name )
				{
					range.first = table.erase(range.first);
				} else
				{
					range.first++;
				}
			}
		} else
		{
			throw std::runtime_error("Unable to find channel.");
		}
	}

	void NetworkCache::setTopic(const std::string &channelName,
								const std::string &topic)
	{
		ChannelCache *channel = m_impl->channel(channelName);
		channel->setTopic(topic);
	}
}
}
