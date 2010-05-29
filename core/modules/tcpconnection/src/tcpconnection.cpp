#include "../tcpconnection.h"

#include <string.h>
#include <netdb.h>
#include <stdexcept>
#include <sstream>

namespace anp
{
namespace firc
{
	TCPConnection::TCPConnection(
		const std::string &hostname,
		const std::string &port
	)
	{
		TCPConnection::connect(hostname, port);
	}

	TCPConnection::~TCPConnection(void)
	{
		::close(m_socket);
		clean();
	}

	void TCPConnection::clean()
	{
	}

	void TCPConnection::connect(
		const std::string &hostname,
		const std::string &port)
	{
		struct addrinfo hints;
		struct addrinfo *pServInfoFirst = NULL;
		struct addrinfo *pServInfoCurrent = NULL;
		memset((void *)&hints, 0, sizeof(hints));
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM; // TCP
		
		getaddrinfo(hostname.c_str(), port.c_str(), &hints,
					&pServInfoFirst);
		
		if ( pServInfoFirst == NULL )
		{
			throw NetworkException("Failed to getaddrinfo()!");
		}
		
		for ( pServInfoCurrent=pServInfoFirst;
			  pServInfoCurrent != NULL;
			  pServInfoCurrent = pServInfoCurrent->ai_next )
		{
			m_socket = ::socket(pServInfoCurrent->ai_family,
								pServInfoCurrent->ai_socktype,
								pServInfoCurrent->ai_protocol);
			if ( -1 == m_socket )
			{
				// Failed
				throw NetworkException("Failed to create socket file descriptor");
			}
			
			if ( -1 == ::connect(	m_socket,
									pServInfoCurrent->ai_addr,
									pServInfoCurrent->ai_addrlen) )
			{
				continue;
			} else
			{
				// Success
				freeaddrinfo(pServInfoFirst);
				return;
			}
		}
		
		// Failed to connect
		freeaddrinfo(pServInfoFirst);
		throw NetworkException("Failed to connect");
	}

	void TCPConnection::send(const std::string &buffer)
	{
		if ( 0 >= ::send(m_socket, buffer.c_str(), (int)buffer.length(), 0) )
		{
			throw NetworkException("Failed to send()");
		}
	}

	void TCPConnection::receive(int8 *buffer, uint32 bufferSize)
	{
		memset((void *)buffer, 0, bufferSize);
		if ( 0 >= ::recv(m_socket, buffer, bufferSize, 0) )
		{
			throw NetworkException("Failed to recv(), connection closed");
		}
	}

	bool32 TCPConnection::waitForSocket(uint32 timeoutSeconds,
										uint32 timeoutMicroseconds)
	{
		timeval timeout;
		timeout.tv_sec = timeoutSeconds;
		timeout.tv_usec = timeoutMicroseconds;
		fd_set readFileDescriptorSet;
		FD_ZERO(&readFileDescriptorSet);
		FD_SET(m_socket, &readFileDescriptorSet);
		if ( (-1) == ::select(m_socket+1, &readFileDescriptorSet,
						NULL, NULL, &timeout) )
		{
			throw NetworkException("select() returned -1");
		}
		return FD_ISSET(m_socket, &readFileDescriptorSet);
	}

} // namespace firc
} // namespace anp
