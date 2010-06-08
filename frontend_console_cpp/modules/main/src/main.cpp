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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <iostream>
#include <fstream>
#include <sstream>

#include <basedefs.h>
#include <pluginmanager.h>
#include <inetwork.h>
#include <networkcache_userinterface.h>
#include <messageprefix.h>
#include <channelcache.h>
#include <unistd.h> // for Sleep
#include <string.h>
#include <log_singleton.h>
#include <tcpconnection.h>
#include <networkevents.h>
#include <networkeventsubscribers.h>
#include <eventdispatcher.h>
#include <networkfactory.h>

static pthread_mutex_t g_stateMutex;
static anp::uint32 g_state = 0;

class LogFileWriter: public anp::ILogInterface
{
public:
	LogFileWriter(const std::string &filename):
	m_filename(filename)
	{
	}
	void present(const anp::dstring &message)
	{
		std::ofstream fout(m_filename.c_str(),
							std::ios_base::app | std::ios_base::out);
		if ( fout.is_open() )
		{
			fout << message << std::endl;
			fout.close();
		}
	}
private:
	std::string m_filename;
};

class EventHandler: public anp::irc::eventsubscribers::Join,
					public anp::irc::eventsubscribers::Part,
					public anp::irc::eventsubscribers::PrivMsg,
					public anp::irc::eventsubscribers::Topic,
					public anp::irc::eventsubscribers::NumericReply
{
public:
	void receiveEvent(anp::irc::events::Join &event)
	{
		std::cout << "irctest_onJoin: " << event.origin().nick()
			<< " joined " << event.channel() << std::endl;
	}
	void receiveEvent(anp::irc::events::Part &event)
	{
		std::cout << "[main.cpp <-] " << event.origin().nick()
			<< " left channel " << event.channel() << ". ("
			<< event.message() << ")" << std::endl;
	}
	void receiveEvent(anp::irc::events::PrivMsg &event)
	{
		std::cout << "[main.cpp <-] " << event.origin().nick() << ": "
			<< event.message() << std::endl;


		if ( event.message() == "die" )
		{
			// Write state
			pthread_mutex_lock(&g_stateMutex);
			g_state = 1; // QUIT
			pthread_mutex_unlock(&g_stateMutex);
		}

		if ( event.target()[0] == '#' && event.message() == "topic?" )
		{
			const anp::irc::NetworkCacheUserInterface &cache =
				event.network().networkCache();
			anp::irc::ChannelCache channel;
			cache.getChannel(event.target(), channel);
	
			std::stringstream ss;
			ss << "PRIVMSG " << channel.name() << " :The topic for "
				<< channel.name() << " is " << channel.topic() << ".\r\n";
			event.network().sendMessage(ss.str());
		}
	}
	void receiveEvent(anp::irc::events::Topic &event)
	{
		std::cout << "[main.cpp <-] " << event.origin().nick()
			<< " changed the topic for " << event.channel() << " to '"
			<< event.topic() << "'" << std::endl;
	}
	void receiveEvent(anp::irc::events::NumericReply &event)
	{
		std::cout << "[main.cpp <-] Numeric reply "
			<< event.command() << " received. ("
			<< event.origin().prefix() << ')' << std::endl;
		
		if ( event.command() == "376" ) // RPL_ENDOFMOTD
		{
			anp::irc::INetwork &network = event.network();
				
			network.sendMessage("JOIN #my-secret-botdev\r\n");
			network.sendMessage(
						"PRIVMSG #my-secret-botdev :Hello world!\r\n");
		}
	}
};

anp::bool32 waitForSocket(int socket,
						  anp::uint32 timeoutSeconds,
						  anp::uint32 timeoutMicroseconds)
{
	timeval timeout;
	timeout.tv_sec = timeoutSeconds;
	timeout.tv_usec = timeoutMicroseconds;
	fd_set readFileDescriptorSet;
	FD_ZERO(&readFileDescriptorSet);
	FD_SET(socket, &readFileDescriptorSet);
	if ( (-1) == ::select(socket+1, &readFileDescriptorSet,
					NULL, NULL, &timeout) )
	{
		throw anp::NetworkException("select() returned -1");
	}
	return FD_ISSET(socket, &readFileDescriptorSet);
}

int main(int argc, char *argv[])
{	
	std::string serverAddress = "irc.chatjunkies.org",
				serverPort = "6667";

	if ( argc >= 3 )
	{
		serverAddress = argv[1];
		serverPort = argv[2];
	}

	const anp::int8 *pluginNames[] = {
		"./libpluginTest1.so"
	};

	LogFileWriter logFileWriter("frontend_cpp.log");
	anp::LogSingletonHelper log;
	log.addLogInterface(&logFileWriter);
	log("Log initialized.");
	
	pthread_mutex_init(&g_stateMutex, NULL);

	anp::irc::NetworkFactory networkFactory;
	anp::irc::PluginManager pluginManager;
	
	for ( anp::uint32 i=0; i<sizeof(pluginNames)/sizeof(pluginNames[0]);
			i++ )
	{
		pluginManager.loadPlugin(
			pluginNames[i],
			networkFactory,
			0
		);
	}
	
	anp::irc::INetwork *network = networkFactory.openNetwork(serverAddress,
															  serverPort,
															  "firc10",
															  "fircclient10",
															  "Test Name");
	
	anp::uint32 state = 0;

	EventHandler eventHandler;
	network->eventDispatcherJoin().subscribe(&eventHandler);
	network->eventDispatcherPart().subscribe(&eventHandler);
	network->eventDispatcherPrivMsg().subscribe(&eventHandler);
	network->eventDispatcherTopic().subscribe(&eventHandler);
	network->eventDispatcherNumericReply().subscribe(&eventHandler);

	network->runMessageReceiverInThread();
	
	// When the quit command has been received, state will equal 1.
	while ( 0 == state )
	{
		if ( waitForSocket(fileno(stdin), 0, 250000) )
		{
			std::string command;
			std::getline(std::cin, command);
			if ( command == "die" )
			{
				break;
			} else
			{
				network->sendMessage(command+"\r\n");
			}
		}

		// Read state
		pthread_mutex_lock(&g_stateMutex);
		state = g_state;
		pthread_mutex_unlock(&g_stateMutex);
		sleep(1); // Just to conserve CPU..
	}
	
	// Quit
	network->deinit("Time to go! See you!");
	networkFactory.closeNetwork(network);
	log("main.cpp: Successfully disconnected.");
	
	pthread_mutex_destroy(&g_stateMutex);
	
	return 0;
}
