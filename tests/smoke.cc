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
 
#include <scp/PathTraceExtension.h>
#include <scp/MasterIDExtension.h>

#include <systemc>
#include <tlm>


SC_MODULE(test)
{
    SC_CTOR(test){

        scp::PathTraceExtension ext;
        ext.initiator_stamp(this);
        SC_REPORT_INFO("ext test",ext.to_string().c_str());

        ext.initiator_stamp(this);
        ext.stamp(this);
        ext.stamp(this);
        SC_REPORT_INFO("ext test",ext.to_string().c_str());

        scp::MasterIDExtension mid(0x1234);
        mid=0x2345;
        mid&=0xff;
        mid<<=4;
        uint64_t myint=mid+mid;
        myint+=mid;
        if (mid==0x450) {
            SC_REPORT_INFO("ext test", "Success\n");
        } else {
            SC_REPORT_INFO("ext test", "Failour\n");
        }
    }
};

int sc_main(int argc, char **argv)
{
    test test1("test");

    sc_core::sc_start();
    return 0;
}


