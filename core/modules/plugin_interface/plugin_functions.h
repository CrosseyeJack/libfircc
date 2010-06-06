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

#ifndef _PLUGIN_FUNCTIONS_H_
#define _PLUGIN_FUNCTIONS_H_

#include <eventdispatcher.h>
#include <networkevents.h>
#include <networkeventdispatchers.h>

namespace anp
{
namespace firc
{
	class NetworkFactory;
	
	// Abbreviations/prefixes:
	// pf - plugin function, implemented in plugins
	
	class INetworkManagerFrontend;
	class MsgPrefix;


	// Typedefs
	// uint32 pluginInit(void *fircCore)
	typedef uint32 (*PF_pluginInit)(NetworkFactory &, void *appContext);
	
	// void pluginDeinit()
	typedef void (*PF_pluginDeinit)(uint32);
} // namespace firc
} // namespace anp

#endif // _PLUGIN_FUNCTIONS_H_
