/*****************************************************************************
  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.
 ****************************************************************************/

#include <scp/tlm_extensions/initiator_id.h>
#include <scp/tlm_extensions/path_trace.h>
#include <scp/report.h>

#include <systemc>
#include <tlm>

SC_MODULE (test) {
    SC_CTOR (test) {
        scp::tlm_extensions::path_trace ext;
        ext.stamp(this);
        SCP_INFO(SCMOD) << ext.to_string();
        ext.reset();

        ext.stamp(this);
        ext.stamp(this);
        ext.stamp(this);

        SCP_INFO(SCMOD) << ext.to_string();
        ext.reset();

        scp::tlm_extensions::initiator_id mid(0x1234);
        mid = 0x2345;
        mid &= 0xff;
        mid <<= 4;
        uint64_t myint = mid + mid;
        myint += mid;
        if (mid == 0x450) {
            SCP_INFO("ext test") << "Success";
        } else {
            SCP_INFO("ext test") << "Failure";
        }
    }
};

int sc_main(int argc, char** argv) {
    SCP_INFO() << "Constructing design";
    test test1("test");
    SCP_INFO() << "Starting simulation";
    sc_core::sc_start();
    return 0;
}
