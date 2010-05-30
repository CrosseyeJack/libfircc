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

#ifndef _MESSAGESENDER_H_
#define _MESSAGESENDER_H_

#include <basedefs.h>
#include <anp_threading.h>
#include <anp_threadsafequeue.h>
#include <log_singleton.h>
#include <string>

namespace anp
{
namespace firc
{
	class TCPConnection;
	
	class MessageSender
	{
	friend void *threadRunMessageSender(void *arg);
	public:
		MessageSender(TCPConnection &connection);
		virtual ~MessageSender();

		void addMessage(const std::string &message);
		
		void setCooldownTime(uint32 ms);
		
		/**
		 * Forces the monitor to stop, without letting it
		 * send the messages still in queue, if any.
		 */
		void stop();
	private:
		// Should be hidden somehow.. private (for now) or maybe interfaced
		void monitor();
		void log(const anp::dstring &message);
	
		TCPConnection &m_connection;
		threading::ThreadSafeQueue<std::string> m_queue;
		
		threading::ProtectedData<uint32, 5000> m_cooldownTime;
		threading::ProtectedData<bool32, false> m_isDying;
		
		threading::Event m_newMessage;
		threading::Thread m_thread;
		
		LogSingletonHelper m_log;
	};
}
}

#endif // _MESSAGESENDER_H_
