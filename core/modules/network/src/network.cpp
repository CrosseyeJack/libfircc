#include "../network.h"
#include <tokenizer.h>
#include <tcpconnection.h>
#include <messageprefix.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <pcrecpp.h>

namespace anp
{
namespace firc
{
namespace numeric_replies
{
	const int8 *const RPL_NAMREPLY = "353";
}
	static void *threadRunMessageReceiver(void *arg)
	{
		Network *nm = (Network *)arg;
		if ( NULL != nm )
		{
			try
			{
				nm->runMessageReceiver();
			} catch ( std::exception &e )
			{
				std::cout
					<< "Network-MessageReceiverRunner: Exception occured: "
					<< e.what() << std::endl;
			}
		}
		std::cout << "Network: closing thread." << std::endl;
		pthread_exit(0);
	}
	
	Network::Network(const int8 *host, const int8 *port):
	m_state(CONNECTING),
	m_connection(host, port),
	m_messageSender(new MessageSender(m_connection))
	{
		std::string m_outStr;
		
		std::cout << "Successfully connected." << std::endl;
		
		m_state = REGISTERING;
		// Nick
		m_outStr = std::string("NICK ")
			+std::string("fircbot09")
			+std::string("\r\n");
		m_connection.send(m_outStr);
		// User
		m_outStr = std::string("USER ")
			+std::string("fircbot09")
			+std::string(" 0 * :")
			+std::string("Anton Petersson")
			+std::string("\r\n");
		m_connection.send(m_outStr);
		
		//m_networkCache.init(host, "fircbot09");
		m_networkCache.setClientNickName("fircbot09");
		
		
		m_state = CONNECTED;
	}
	
	Network::~Network()
	{
		if ( NULL != m_receiverThread.get() )
		{
			// Ask the thread to exit
			m_stateMutex.lock();
			m_state = SHUTTING_DOWN;
			m_stateMutex.unlock();
			// Now just wait for it to finnish
			m_receiverThread->join(NULL);
			// (Implicitly) kill the connection
		}
	}
	
	/**
	Sends the QUIT message to the server and waits (blocking)
	for the receiver thread to finish.

	You still need to destroy the Network object
	after calling this function.
	*/
	void Network::deinit(const std::string &message)
	{
		m_stateMutex.lock();
		m_state = SHUTTING_DOWN;
		m_stateMutex.unlock();
		
		std::string quitMessage("QUIT :");
		quitMessage += message;
		quitMessage += "\r\n";
		m_connection.send(quitMessage);
		
		if ( NULL != m_receiverThread.get() )
		{
			m_receiverThread->join(NULL);
			m_receiverThread.reset(NULL);
		}
	}

	/**
	Creates a thread to be able to run the message receiver
	without blocking.

	Throws an exception if the thread cannot be created.
	*/
	void Network::runMessageReceiverInThread()
	{
		m_receiverThread.reset(new threading::Thread);
		m_receiverThread->create(NULL,	threadRunMessageReceiver,
								(void *)this);
	}
	
	/**
	Runs the message receiver, doesn't return until disconnected.
	*/
	Result Network::runMessageReceiver()
	{
		using tokenizer::tokenize;
		State state = UNKNOWN;
		bool32 connected = true;

		char buffer[MAX_DATA_LENGTH];
		bool hasPrefix=false;
		std::string in,
					currentMessage,
					prefix,
					command,
					leftOvers,
					target,
					sender,
					temp1,
					temp2,
					temp3;
					
		std::cout << "runMessageReceiver():start" << std::endl;
		
		while ( TRUE == connected )
		{
			//sleep(2);
			//std::cout << "messageReceiver: hi" << std::endl;
			
			m_stateMutex.lock();
			state = m_state;
			m_stateMutex.unlock();
			
			if ( state == SHUTTING_DOWN )
			{
				break;
			}
			
			
			/////////////////////////////////////////

			// 1. Receive a message
			if ( m_connection.waitForSocket(0, 500000) )
			{
				if ( RES_CONNECTION_CLOSED == m_connection.receive(
						buffer, sizeof(buffer)/sizeof(buffer[0])) )
				{
					// The server closed the connection
					// Flag we're not connected anymore, and exit thread
					connected = FALSE;
					return RES_CONNECTION_CLOSED;
				}
			} else
			{
				// There was no data available from the server
				// check again
				continue;
			}
			in = buffer; // optimize..?
			//std::cout << "<- " << in << std::endl;

			// 2. Tokenize it and parse it
			// Start by dividing it up into several messages
			// (sometimes the server
			// will send several messages at the same time)
			while ( 1 ) {
				if ( tokenize(currentMessage, in, "\r\n") ) {
					// if there was something left from the previous
					// parsing run, add it here
					if ( leftOvers.length() > 0 ) {
						currentMessage = leftOvers + currentMessage;
						leftOvers.erase();
					}
					std::cout << "<- " << currentMessage << std::endl;
					parseMessage(currentMessage);
					/*
					// Reset old vars as needed
					prefix.erase();
					// Start parsing the current message
					if ( currentMessage[0] == ':' ) {
						// The message has a prefix
						currentMessage.erase(0, 1);
						// Get the prefix
						tokenize(prefix, currentMessage, ' ');
					}
					// Get the command
					tokenize(command, currentMessage, ' ');

					if ( command == "PING" ) {
						 // Strip the starting colon ':'
						currentMessage.erase(0, 1);
						temp3 = "PONG :";
						temp3 += currentMessage;
						temp3 += "\r\n";
						std::cout << "receiver: Sending mgs=" << temp3
							<< std::endl;
						m_connection.send(temp3);
						//#m_pluginManager->onPing(currentMessage);
					} else if ( command == "PRIVMSG" ) {
						tokenize(target, currentMessage, ' ');

						currentMessage.erase(0, 1);
						std::cout << "(local) PRIVMSG detected: "
							<< currentMessage << std::endl;

						PrivMsgJob job(	NULL,
										*this,
										prefix.c_str(),
										target.c_str(),
										currentMessage.c_str());
						m_pluginManager->performJob(&job,
											PluginManager::IRC_PRIVMSG);
					} else if ( command == "JOIN" ) {
						std::cout << "(local) JOIN command detected: "
							<< currentMessage << std::endl;
						if ( currentMessage[0] == ':' )
						{
 							// Remove ':' in front of the channel
 							// name (efnet seems to use it)
							currentMessage.erase(0, 1);
						}

						// Separate the prefix into nickname,
						// user and host
						tokenize(temp1, prefix, '!'); // nickname
						tokenize(temp2, prefix, '@'); // user
						// prefix now only holds the host

						//m_networkCache.onJoin(temp1,
						//					  temp2,
						//					  prefix,
						//					  currentMessage);
						std::string clientNick;
						m_networkCache.getClientNickName(clientNick);
						if ( temp1 == clientNick )
						{
							m_networkCache.addChannel(currentMessage);
						}
						m_networkCache.addUserToChannel(temp1, temp2,
												prefix, currentMessage);
												
						JoinJob joinJob(NULL,
										(void *)this,
										currentMessage.c_str(),
										temp1.c_str());
						m_pluginManager->performJob(&joinJob,
											PluginManager::IRC_JOIN);
						std::cout << "Join command handled successfully."
															<< std::endl;
					} else if ( command == "PART" ) {
						if ( currentMessage[0] == ':' )
						{
						 	// Remove ':' in front of the channel name
							currentMessage.erase(0, 1);
						}
						// Store channelname into temp3
						// currentMessage will hold part message, if any
						tokenize(temp3, currentMessage, " :");

						// Separate the prefix into nickname, user
						// and host (UNNECESSARY FOR PART MESSAGE !?
						//  ONLY USES NICKNAME ANYWAY!!!)
						tokenize(temp1, prefix, '!'); // nickname
						tokenize(temp2, prefix, '@'); // user
						// prefix now only holds the host

						//m_networkCache.onPart(temp1,
						//					  temp2,
						//					  prefix,
						//					  currentMessage);

						m_networkCache.removeUserFromChannel(temp1,
																temp3);
						std::string clientNick;
						m_networkCache.getClientNickName(clientNick);
						if ( temp1 == clientNick )
						{
							m_networkCache.removeChannel(currentMessage);
						}

						//#m_pluginManager->onPart(temp1,
						// currentMessage);
					} else if ( command == "TOPIC" ) {
						// topic change.. ex: (GTANet)
						// [Original message]
						// ":Pliskin!IceChat7@gtanet-ADE03D4F.bsnet.se
						// TOPIC #antons_kanal :Det här är en topic"
						tokenize(target, currentMessage, " :");

						m_networkCache.setTopic(target, currentMessage);
						//m_pluginManager->onTopicChange(); TODO

						// ##### NUMERIC REPLIES BELOW ######
					} else if ( command == "332" ) {
						// RPL_TOPIC ex: (GTANet)
						//[Original message]
						// ":frosties.de.eu.gtanet.com 332
						// firc_04 #antons_kanal :lalala"
						
						 // target = firc_04
						tokenize(target, currentMessage, ' ');
						 // temp1 = #antons_kanal
						tokenize(temp1, currentMessage, " :");
						
						m_networkCache.setTopic(temp1, currentMessage);
					} else if ( command == "333" ) {
						// "RPL_TOPICWHOWHEN" ? Ex: (ChatJunkies)
						// [original message] :at.chatjunkies.org 333
						// fircbot09 #my-secret-botdev
						// antonp!~antonp@ChatJunkies-6a09bfa2.dyn.
						// perspektivbredband.net 1248007592

					} else if ( command == "353" ) {
						// RPL_NAMREPLY ex: (efnet) [Original message]
						// :efnet.demon.co.uk 353 firc_04 =
						// #antons_kanal :firc_04 @Pliskin_
						
						 // target = firc_04
						tokenize(target, currentMessage, ' ');
						 // temp1 = '='
						tokenize(temp1, currentMessage, ' ');
						 // temp2 = #antons_kanal
						tokenize(temp2, currentMessage, " :");

						bool keepGoing = true;
						while ( keepGoing ) {
							// temp3 = [@|+|nothing]nickname
							keepGoing = tokenize(temp3,
												 currentMessage,
												 " ");
							switch ( temp3[0] ) {
							case '@':
								temp3.erase(0, 1);
								break;
							case '+':
								temp3.erase(0, 1);
								break;
							default:
								break;
							}
							//m_networkCache.addUser(temp2, temp3);
							m_networkCache.addUserToChannel(temp3, "", "",
																 temp2);
						}
					}*/
				} else {
					// The message isn't complete, so save it and
					// add the rest when it arrives [todo]
					//m_out << "[Parsing results] The message wasn't
					// complete. (Looked like: " << currentMessage
					// << ")\n";
					if ( currentMessage.length() >= 3 )
					{
						leftOvers = currentMessage.erase(
							currentMessage.length()-3);
					}
					break;
				}
				if ( in.length() == 0 )
					break;
			}			
		}
		return RES_OK;
	}

	void parseParams(std::string &all, std::string list[])
	{
		using std::cout;
		using std::endl;
		using std::string;
		using tokenizer::tokenize;
		// Assuming list holds 15 elements
		cout << endl << "all params: " << all << endl;

		for ( int i=0; i<15; i++ )
		{
			if ( all[0] == ':' )
			{
				list[i] = all.substr(1);
				break;
			} else if ( !tokenize(list[i], all, " ") )
			{
				break;
			}
		}
	}

	void Network::parseMessage(const std::string &message)
	{
		using pcrecpp::RE;

		std::string prefix, command, parameters, params[15];
		std::string nick, user, host;

		static
		RE ircMsgPattern(
			"^(:(\\S+?)\\s)?(\\S+)\\s?(:?.*)$"
		);
		static
		RE prefixPattern("^(\\S+?)!(\\S+?)@(\\S+?)$");
		static
		RE numPattern("^[[:digit:]]{3}$");

		bool32 validMessage = ircMsgPattern.FullMatch(message, (void *)0,
			&prefix, &command, &parameters);

		if ( validMessage )
		{
			bool32 validUser = prefixPattern.FullMatch(prefix,
														&nick,
														&user,
														&host);
			MsgPrefix msgPrefix(prefix, nick, user, host);

			parseParams(parameters, params);

			if ( numPattern.FullMatch(command) )
			{
				// Numeric replies
				if ( command == numeric_replies::RPL_NAMREPLY )
				{
					msgNumHandleRPL_NAMREPLY(params[2], params[3]);
				}
				msgNumHandle(msgPrefix, command, params);
			} else
			{
				if ( command == "PING" )
				{
					msgPingHandle(params[0], "");
				} else if ( command == "JOIN" )
				{
					msgJoinHandle(msgPrefix, params[0]);
				} else if ( command == "PART" )
				{
					msgPartHandle(msgPrefix, params[0],
									params[1]);
				} else if ( command == "PRIVMSG" )
				{
					msgPrivMsgHandle(msgPrefix, params[0],
										params[1]);
				} else if ( command == "TOPIC" )
				{
					msgTopicHandle(msgPrefix, params[0],
										params[1]);
				}
				// TODO
				//msgCommandHandle(msgPrefix, command, params);
			}

		} else
		{
			std::cout << "(lib)Invalid IRC message: " << message
				<< "(" << "p=" << prefix << "c=" << command << "pa="
				<< parameters << ")" << std::endl;
			// unknownMessageHandler()!
		}
	}

	void Network::msgPingHandle(const std::string &server1,
										const std::string &server2)
	{
		std::string pong = "PONG :";
		pong += server1 + "\r\n";
		std::cout << "-> " << pong << std::endl;
		m_connection.send(pong);
	}

	void Network::msgJoinHandle( const MsgPrefix &origin,
										const std::string &channel)
	{
		std::string clientNick;
		m_networkCache.getClientNickName(clientNick);
		if ( origin.nick() == clientNick )
		{
			m_networkCache.addChannel(channel);
		}
		m_networkCache.addUserToChannel(origin.nick(),
										origin.user(),
										origin.host(),
										channel);
		events::Join event(*this, origin, channel);
		m_eventDispatchers.join.dispatch(event);

//		JoinJob joinJob(NULL,
//						*this,
//						origin,
//						channel.c_str());
//		m_pluginManager->performJob(&joinJob, PluginManager::IRC_JOIN);
	}

	void Network::msgPartHandle(	const MsgPrefix &origin,
										const std::string &channel,
										const std::string &message)
	{

		std::string clientNick;
		m_networkCache.getClientNickName(clientNick);
		if ( origin.nick() == clientNick )
		{
			m_networkCache.removeChannel(channel);
		} else
		{
			m_networkCache.removeUserFromChannel(origin.nick(),
												channel);
		}

		events::Part event(*this, origin, channel, message);
		m_eventDispatchers.part.dispatch(event);

//		PartJob partJob(NULL,
//						*this,
//						origin,
//						channel.c_str(),
//						message.c_str());
//		m_pluginManager->performJob(&partJob, PluginManager::IRC_PART);
	}
	
	void Network::msgPrivMsgHandle(	const MsgPrefix &origin,
											const std::string &target,
											const std::string &message)
	{
		events::PrivMsg event(*this, origin, target, message);
		m_eventDispatchers.privMsg.dispatch(event);

//		PrivMsgJob job(	NULL,
//						*this,
//						origin,
//						target.c_str(),
//						message.c_str());
//		m_pluginManager->performJob(&job, PluginManager::IRC_PRIVMSG);
	}
	
	void Network::msgTopicHandle(const MsgPrefix &origin,
										const std::string &channel,
										const std::string &topic)
	{
		m_networkCache.setTopic(channel, topic);

		events::Topic event(*this, origin, channel, topic);
		m_eventDispatchers.topic.dispatch(event);

//		TopicJob job(	NULL,
//						*this,
//						origin,
//						channel,
//						topic);
//		m_pluginManager->performJob(&job, PluginManager::IRC_TOPIC);
	}

	void Network::msgNumHandleRPL_NAMREPLY(
								const std::string &channel,
								const std::string &userlist)
	{
		using tokenizer::tokenize;
		std::string nick, userlistCopy = userlist;
		bool keepGoing = true;

		while ( keepGoing ) {
			// nick = [@|+|nothing]nickname
			keepGoing = tokenize(nick, // note, reusing var
								 userlistCopy,
								 " ");
			switch ( nick[0] ) {
			case '@':
				nick.erase(0, 1);
				break;
			case '+':
				nick.erase(0, 1);
				break;
			default:
				break;
			}
			// Stupid temporary fix.
			// Parsing should be fixed instead.
			if ( nick != "" && nick != " " )
			{
				m_networkCache.addUserToChannel(nick, "", "",
												 channel);
			}
		}
	}

	void Network::msgNumHandle(const MsgPrefix &origin,
										const std::string &command,
										const std::string params[])
	{
		events::NumericReply event(*this, origin, command, params);
		m_eventDispatchers.num.dispatch(event);
	}
	
	void Network::sendMessage(const std::string &message)
	{
		m_messageSender->addMessage(message);
	}

	const NetworkCacheUserInterface &Network::networkCache() const
	{
		return m_networkCache;
	}

	IEventDispatcherSubscriber<events::ISubscriber<events::Join> > &
	Network::eventDispatcherJoin()
	{
		return m_eventDispatchers.join;
	}

	IEventDispatcherSubscriber<events::ISubscriber<events::Part> > &
	Network::eventDispatcherPart()
	{
		return m_eventDispatchers.part;
	}

	IEventDispatcherSubscriber<events::ISubscriber<events::PrivMsg> > &
	Network::eventDispatcherPrivMsg()
	{
		return m_eventDispatchers.privMsg;
	}

	IEventDispatcherSubscriber<events::ISubscriber<events::Topic> > &
	Network::eventDispatcherTopic()
	{
		return m_eventDispatchers.topic;
	}

	IEventDispatcherSubscriber<
		events::ISubscriber<events::NumericReply>
	> &
	Network::eventDispatcherNumericReply()
	{
		return m_eventDispatchers.num;
	}

} // namespace firc
} // namespace anp
