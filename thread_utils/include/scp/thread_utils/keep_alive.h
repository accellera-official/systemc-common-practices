/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

/**
 * @class KeepAlive
 * @brief this class is used to keep the simulation alive by using an async
 * event
 */
#ifndef _SCP_KEEP_ALIVE_H_
#define _SCP_KEEP_ALIVE_H_

#include <systemc>
#include <scp/thread_utils/async_event.h>

namespace scp {
class keep_alive : public sc_core::sc_module
{
public:
    scp::scp_async_event keep_alive_event;

    keep_alive(sc_core::sc_module_name name): sc_core::sc_module(name) {
        keep_alive_event.async_attach_suspending();
    }
    void finish() {
        keep_alive_event.async_detach_suspending();
        keep_alive_event.notify();
    }
    ~keep_alive() {}
};
} // namespace scp

#endif
