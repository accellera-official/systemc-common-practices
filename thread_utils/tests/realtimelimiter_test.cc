/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All Rights Reserved.
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

#include <systemc>
#include <scp/thread_utils/real_time_limiter.h>

#include <unistd.h>

SC_MODULE (runner) {
    void thd() {
        while (1) {
            wait(1234, sc_core::SC_MS);
        }
    }
    SC_CTOR (runner) {
        SC_THREAD(thd);
    }
};

int sc_main(int argc, char** argv) {
    runner r("runner");
    scp::real_time_limiter rtl("Realtimelimiter");

    auto startRT = std::chrono::high_resolution_clock::now();

    sc_core::sc_start(5, sc_core::SC_SEC);
    auto rtime = sc_core::sc_time(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - startRT)
            .count(),
        sc_core::SC_US);

    sc_core::sc_stop();
    std::cout << "SystemC time " << sc_core::sc_time_stamp()
              << " and real time " << rtime.to_seconds() << "s \n"
              << std::flush;
    return (!(std::round(sc_core::sc_time_stamp().to_seconds()) ==
              std::round(rtime.to_seconds())));
}
