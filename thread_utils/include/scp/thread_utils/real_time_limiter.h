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

#ifndef _SCP_REALTIMLIMITER_H
#define _SCP_REALTIMLIMITER_H

#include <thread>
#include <mutex>
#include <queue>
#include <future>

#include <systemc>
#include <scp/thread_utils/async_event.h>

namespace scp {
/**
 * @brief realtimelimiter: sc_module which suspends SystemC if SystemC time
 * drifts ahead of realtime
 * @param @RTquantum_ms : realtime tick rate between checks.
 * @param @SCTimeout_ms : If SystemC time is behind by more than this value,
 * then generate a fatal abort (0 disables)
 *
 */

SC_MODULE (real_time_limiter) {
    std::chrono::high_resolution_clock::time_point startRT;
    sc_core::sc_time startSC;
    sc_core::sc_time runto;
    std::thread m_tick_thread;
    bool running = false;
    bool suspended = false;
    sc_core::sc_time suspend_at = sc_core::SC_ZERO_TIME;
    scp::scp_async_event tick;

    unsigned int m_RTquantum_ms = 100;

    void SCticker() {
        if (!running) {
            tick.async_detach_suspending();
            sc_core::sc_unsuspend_all();
            return;
        }
        if (suspended == true && (sc_core::sc_time_stamp() > suspend_at)) {
            /* If we were suspended, and time has drifted, then account for it
             * here. */
            sc_core::sc_time drift = sc_core::sc_time_stamp() - suspend_at;
            startSC += drift;
        }

        if (sc_core::sc_time_stamp() >= runto) {
            tick.async_attach_suspending();
            sc_core::sc_suspend_all(); // Dont starve while we're waiting
                                       // for realtime.
            suspend_at = sc_core::sc_time_stamp();
            suspended = true;
        } else {
            suspended = false;
            // lets go with +=1/2 a RTquantum
            tick.notify(
                (runto +
                 sc_core::sc_time(m_RTquantum_ms, sc_core::SC_MS) / 2) -
                sc_core::sc_time_stamp());
            tick.async_detach_suspending(); // We could even starve !
            sc_core::sc_unsuspend_all();
        }
    }

    void RTticker() {
        while (running) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(m_RTquantum_ms));

            runto = sc_core::sc_time(
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::high_resolution_clock::now() -
                            startRT)
                            .count(),
                        sc_core::SC_US) +
                    startSC;

            tick.notify();
        }
    }

    void end_of_simulation() {
        disable();
    }

public:
    /* NB all these functions should be called from the SystemC thread of
     * course*/
    void enable() {
        assert(running == false);

        running = true;

        startRT = std::chrono::high_resolution_clock::now();
        startSC = sc_core::sc_time_stamp();
        runto = sc_core::sc_time(m_RTquantum_ms, sc_core::SC_MS) +
                sc_core::sc_time_stamp();
        tick.notify(sc_core::sc_time(m_RTquantum_ms, sc_core::SC_MS));

        m_tick_thread = std::thread(&real_time_limiter::RTticker, this);
    }

    void disable() {
        if (running) {
            running = false;
            m_tick_thread.join();
        }
    }
    real_time_limiter(
        const sc_core::sc_module_name& name, bool autostart = true,
        sc_core::sc_time accuracy = sc_core::sc_time(100, sc_core::SC_MS)):
        sc_module(name),
        tick(false),
        m_RTquantum_ms((accuracy.to_seconds()) * 1000.0) {
        SC_METHOD(SCticker);
        dont_initialize();
        sensitive << tick;
        if (autostart) {
            SC_THREAD(enable);
        }
    }
};
} // namespace scp

#endif // _SCP_REALTIMLIMITER_H
