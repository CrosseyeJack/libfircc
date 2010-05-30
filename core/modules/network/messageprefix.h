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

#ifndef _MESSAGE_PREFIX_H_
#define _MESSAGE_PREFIX_H_

#include <string>

namespace anp
{
namespace firc
{
class MsgPrefix
{
public:
	MsgPrefix(	const std::string &prefix,
				const std::string &nick,
				const std::string &user,
				const std::string &host):
	m_prefix(prefix),
	m_nick(nick),
	m_user(user),
	m_host(host)
	{
	}

	const std::string &prefix() const { return m_prefix; }
	const std::string &nick() const { return m_nick; }
	const std::string &user() const { return m_user; }
	const std::string &host() const { return m_host; }
protected:
	std::string m_nick;
	std::string m_user;
	std::string m_host;
	std::string m_prefix;
};
} // namespace firc
} // namespace anp

#endif // _MESSAGE_PREFIX_H_
