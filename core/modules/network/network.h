/*

Copyright (c) 2010, Anton Petersson < anton (at) anp.nu >
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _NETWORKMANAGER_H_
#define _NETWORKMANAGER_H_

#include <basedefs.h>
#include <tcpconnection.h>
#include <networkcache.h>
#include "inetwork.h"
#include "inc/messagesender.h"
#include <eventdispatcher.h>
#include <networkevents.h>
#include <log_singleton.h>

#include <anp_threading.h> // hide?
#include <memory>

namespace anp
{
namespace firc
{
	class PluginManager;
	class MsgPrefix;
	
	class Network: public INetwork
	{
	public:
		Network(const int8 *host, const int8 *port);
		~Network();
		
		void deinit(const std::string &message);
		enum State
		{
			CONNECTING=0,
			REGISTERING,
			CONNECTED,
			SHUTTING_DOWN,
			UNKNOWN
		};
		// Non-blocking, creates a thread on it's own
		void runMessageReceiverInThread();
		// Blocking
		void runMessageReceiver();
		
		void sendMessage(const std::string &message);
		
		// Network cache stuff
		const NetworkCacheUserInterface &networkCache() const;
		Result getBotNickName(int8 *destName,
								uint32 maxNameLength) const;

		// Event stuff
		dispatchers::Join &
			eventDispatcherJoin();

		dispatchers::Part &
			eventDispatcherPart();

		dispatchers::PrivMsg &
			eventDispatcherPrivMsg();

		dispatchers::Topic &
			eventDispatcherTopic();

		dispatchers::NumericReply &
			eventDispatcherNumericReply();
			
		dispatchers::Ping &
			eventDispatcherPing();

	private:
		void parseMessage(const std::string &message);
		void msgPingHandle( const MsgPrefix &origin,
							const std::string &server1,
							const std::string &server2);
		void msgJoinHandle(	const MsgPrefix &origin,
							const std::string &channel);
		void msgPartHandle(	const MsgPrefix &origin,
							const std::string &channel,
							const std::string &message);
		void msgPrivMsgHandle(	const MsgPrefix &origin,
								const std::string &target,
								const std::string &message);
		void msgTopicHandle(const MsgPrefix &origin,
							const std::string &channel,
							const std::string &topic);
		void msgNumHandleRPL_NAMREPLY(
								const std::string &channel,
								const std::string &userlist);
		void msgNumHandle(const MsgPrefix &origin,
							const std::string &command,
							const std::string params[]);

		State m_state;
		threading::Mutex m_stateMutex;
		
		TCPConnection m_connection;
		
		std::auto_ptr<threading::Thread> m_receiverThread;
		std::auto_ptr<MessageSender> m_messageSender;
		
		NetworkCache m_networkCache;

		struct
		{
			EventDispatcher<
				ISubscriber<events::Join>,
				events::Join
			> join;
			EventDispatcher<
				ISubscriber<events::Part>,
				events::Part
			> part;
			EventDispatcher<
				ISubscriber<events::PrivMsg>,
				events::PrivMsg
			> privMsg;
			EventDispatcher<
				ISubscriber<events::Topic>,
				events::Topic
			> topic;
			EventDispatcher<
				ISubscriber<events::NumericReply>,
				events::NumericReply
			> num;
			EventDispatcher<
				ISubscriber<events::Ping>,
				events::Ping
			> ping;
		} m_eventDispatchers;
		
		LogSingletonHelper m_log;
	};
} // namespace firc
} // namespace anp

#endif // _NETWORKMANAGER_H_
