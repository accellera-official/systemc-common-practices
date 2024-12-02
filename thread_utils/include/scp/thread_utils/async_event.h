/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All Rights
 * Reserved.
 * Author: GreenSocs 2022
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

#ifndef _SCP_ASYNC_EVENT
#define _SCP_ASYNC_EVENT

#include <iostream>

#include <systemc>
#include <tlm>
#include <thread>
#include <mutex>

namespace scp {
class scp_async_event : public sc_core::sc_prim_channel,
                        public sc_core::sc_event
{
private:
    sc_core::sc_time m_delay;
    std::thread::id tid;
    std::mutex mutex; // Belt and braces
    bool outstanding=false;

public:
    scp_async_event(bool start_attached = true) {
        tid = std::this_thread::get_id();
        enable_attach_suspending(start_attached);
    }

    void async_notify() { notify(); }

    void notify() {
        if (tid == std::this_thread::get_id()) {
            sc_core::sc_event::notify();
        } else {
            notify(sc_core::SC_ZERO_TIME);
        }
    }
    void notify(double d, sc_core::sc_time_unit u) {
        notify(sc_core::sc_time(d, u));
    }
    void notify(sc_core::sc_time delay) {
        if (tid == std::this_thread::get_id()) {
            sc_core::sc_event::notify(delay);
        } else {
            mutex.lock();
            m_delay = delay;
            outstanding = true;
            mutex.unlock();
            async_request_update();
        }
    }

    bool triggered() const {
        if (tid == std::this_thread::get_id()) {
            return sc_core::sc_event::triggered();
        } else {
            SC_REPORT_ERROR("scp_async_event",
                            "It is an error to call triggered() from outside "
                            "the SystemC thread");
        }
        return false;
    }
    void async_attach_suspending() {
        this->sc_core::sc_prim_channel::async_attach_suspending();
    }

    void async_detach_suspending() {
        this->sc_core::sc_prim_channel::async_detach_suspending();
    }

    void enable_attach_suspending(bool e) {
        e ? this->async_attach_suspending() : this->async_detach_suspending();
    }

private:
    void update(void) {
        mutex.lock();
        // we should be in SystemC thread
        if (outstanding) {
            sc_event::notify(m_delay);
            outstanding = false;
        }
        mutex.unlock();
    }
    void start_of_simulation() {
        // we should be in SystemC thread
        if (outstanding) {
            request_update();
        }
    }
};
} // namespace scp

#endif // SCP_ASYNC_EVENT
