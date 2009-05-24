#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

#include <basedefs.h>
//#include <windows.h>
//#include <winsock.h>
#include <cstdio>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

#define MAX_HOST_LENGTH 256
#define MAX_DATA_LENGTH 2048
#define N_ERROR_HOSTNAME_TOO_LONG -1
#define N_ERROR_INVALID_HOST -2
#define N_ERROR_SOCKET_INVALID -3
#define N_ERROR_PORT_NOT_SET -4
#define N_ERROR_CONNECTION_FAILED -5
#define N_CONNECTION_CLOSED -6
#define N_OK 1

namespace firc
{
	class TCPConnection
	{
	public:
		TCPConnection();
		TCPConnection(const std::string &hostname,
						const std::string &port);
		virtual ~TCPConnection();
		Result connect(
			const std::string &hostname,
			const std::string &port
		);
		Result send(const std::string &buffer);
		Result receive(int8 *buffer, uint32 bufferSize);
		void closeSocket();
		void clean();
		int getLastError();
	private:
		int m_socket;
	};
}
#endif
