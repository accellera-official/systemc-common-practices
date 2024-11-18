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
#include <scp/thread_utils/keep_alive.h>
#include <scp/thread_utils/run_on_systemc.h>

#include <unistd.h>

int sc_main(int argc, char** argv) {
    scp::keep_alive my_keep_alive("MyKeepAlive");
    scp::run_on_systemc rsc;
    std::thread t([&]() {
        sleep(2);
        rsc.scp_run_on_systemc([&]() {
            wait(42, sc_core::SC_SEC);
            my_keep_alive.finish();  // Remove the keep alive from the SystemC thread!
        });
    });
    sc_core::sc_start();
    std::cout << "SystemC time " << sc_core::sc_time_stamp() << "\n"
              << std::flush;
    t.join();
    return (!(sc_core::sc_time_stamp().to_seconds() == 42));
}
