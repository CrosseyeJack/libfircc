
#include <fircc/inetwork.h>

class NetworkTestImpl: public anp::irc::INetwork
{
public:
    NetworkTestImpl() { }
    virtual ~NetworkTestImpl() { }

    virtual void deinit(const std::string &message) { }
    virtual void sendMessage(const std::string &message) { }
    virtual void runMessageReceiverInThread() { }
    virtual void runMessageReceiver() { }
    virtual void tryReceive() { }
    virtual int addSocketToFdSet(fd_set *readfds) { return 0; }
    virtual bool internalSocketInSet(fd_set *fds) { return false; }
    virtual const std::string &host() { return m_todo; }
    virtual const std::string &port() { return m_todo; }
    virtual const anp::irc::NetworkCacheUserInterface &networkCache() const { }
    virtual anp::irc::dispatchers::Join &eventDispatcherJoin()
    {
        return m_join;
    }
    virtual anp::irc::dispatchers::Part &eventDispatcherPart()
    {
        return m_part;
    }
    virtual anp::irc::dispatchers::PrivMsg &eventDispatcherPrivMsg()
    {
        return m_privmsg;
    }
    virtual anp::irc::dispatchers::Topic &eventDispatcherTopic()
    {
        return m_topic;
    }
    virtual anp::irc::dispatchers::Kick &eventDispatcherKick()
    {
        return m_kick;
    }
    virtual anp::irc::dispatchers::Quit &eventDispatcherQuit()
    {
        return m_quit;
    }
    virtual anp::irc::dispatchers::Nick &eventDispatcherNick()
    {
        return m_nick;
    }
    virtual anp::irc::dispatchers::NumericReply &eventDispatcherNumericReply()
    {
        return m_numericreply;
    }
    virtual anp::irc::dispatchers::Command &eventDispatcherCommand()
    {
        return m_command;
    }
    virtual anp::irc::dispatchers::Ping &eventDispatcherPing()
    {
        return m_ping;
    }
    virtual anp::irc::dispatchers::ExceptionOccured &
        eventDispatcherExceptionOccured()
    {
        return m_exceptionoccured;
    }

    anp::EventDispatcher<
        anp::ISubscriber<anp::irc::events::Join>,
        anp::irc::events::Join
    > m_join;

    anp::EventDispatcher<
        anp::ISubscriber<anp::irc::events::Part>,
        anp::irc::events::Part
    > m_part;

    anp::EventDispatcher<
        anp::ISubscriber<anp::irc::events::PrivMsg>,
        anp::irc::events::PrivMsg
    > m_privmsg;
    
    anp::EventDispatcher<
        anp::ISubscriber<anp::irc::events::Topic>,
        anp::irc::events::Topic
    > m_topic;

    anp::EventDispatcher<
        anp::ISubscriber<anp::irc::events::Kick>,
        anp::irc::events::Kick
    > m_kick;

    anp::EventDispatcher<
        anp::ISubscriber<anp::irc::events::Quit>,
        anp::irc::events::Quit
    > m_quit;

    anp::EventDispatcher<
        anp::ISubscriber<anp::irc::events::Nick>,
        anp::irc::events::Nick
    > m_nick;

    anp::EventDispatcher<
        anp::ISubscriber<anp::irc::events::NumericReply>,
        anp::irc::events::NumericReply
    > m_numericreply;

    anp::EventDispatcher<
        anp::ISubscriber<anp::irc::events::Command>,
        anp::irc::events::Command
    > m_command;
    
    anp::EventDispatcher<
        anp::ISubscriber<anp::irc::events::Ping>,
        anp::irc::events::Ping
    > m_ping;

    anp::EventDispatcher<
        anp::ISubscriber<anp::irc::events::ExceptionOccured>,
        anp::irc::events::ExceptionOccured
    > m_exceptionoccured;
private:
    std::string m_todo; // TODO: Just to keep warnings away until this is reworked
};

