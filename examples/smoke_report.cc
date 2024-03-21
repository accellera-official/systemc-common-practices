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
#include <cci_configuration>

#include <cstdio>
#include <string>
#include <fstream>
#include <streambuf>
#include <unistd.h>

SC_MODULE (test4) {
    SC_CTOR (test4) {
        SCP_INFO(()) << " .   T4 Logger() 1";
        SCP_WARN(()) << " .   T4 Logger() 1";
        SCP_INFO(()) << " .   T4 Logger() 2";
        SCP_WARN(()) << " .   T4 Logger() 2";
    }
    SCP_LOGGER();
};

SC_MODULE (test3) {
    SC_CTOR (test3) {
        SCP_INFO((D)) << " .  T3 D Logger \"other\" \"feature.one\"";
        SCP_WARN((D)) << " .  T3 D Logger \"other\" \"feature.one\"";
        SCP_INFO(()) << " .  T3 Logger ()";
        SCP_WARN(()) << " .  T3 Logger ()";
    }
    SCP_LOGGER((D), "other", "feature.one");
    SCP_LOGGER(());
};

SC_MODULE (test2) {
    SC_CTOR (test2) : t31("t3_1"), t32("t3_2"), t4("t4") {
            SCP_INFO(()) << "  T2 Logger()";
            SCP_WARN(()) << "  T2 Logger()";
        }
    SCP_LOGGER();
    test3 t31, t32;
    test4 t4;
};

SC_MODULE (test1) {
    SC_CTOR (test1) : t2("t2") {
            SCP_WARN((), "My.Name") << " T1 My.Name typed log";
            SCP_INFO(()) << " T1 Logger()";
            SCP_WARN(()) << " T1 Logger()";

            SCP_LOGGER_VECTOR_PUSH_BACK(vec, "some", "thing1");
            SCP_LOGGER_VECTOR_PUSH_BACK(vec, "some", "thing2");

            SCP_INFO((vec[0])) << "Thing1?";
            SCP_WARN((vec[0])) << "Thing1?";
            SCP_INFO((vec[1])) << "Thing2?";
            SCP_WARN((vec[1])) << "Thing2?";
        }
    SCP_LOGGER("something", "else");
    SCP_LOGGER_VECTOR(vec);
    test2 t2;
};

class outside_class
{
    SCP_LOGGER("out.class", "thing1");

public:
    outside_class() {
        SCP_INFO(())("constructor");
        SCP_WARN(())("constructor");
    }
};

SC_MODULE (test) {
    outside_class oc;
    SC_CTOR (test) {
        SCP_DEBUG(SCMOD) << "First part";
        scp::tlm_extensions::path_trace ext;
        ext.stamp(this);
        SCP_INFO(SCMOD) << ext.to_string();
        ext.reset();

        ext.stamp(this);
        ext.stamp(this);
        ext.stamp(this);

        SCP_INFO(SCMOD) << ext.to_string();
        ext.reset();

        SCP_DEBUG(SCMOD) << "Second part";
        scp::tlm_extensions::initiator_id mid(0x1234);
        mid = 0x2345;
        mid &= 0xff;
        mid <<= 4;
        uint64_t myint = mid + mid;
        myint += mid;
        if (mid == 0x450) {
            SC_REPORT_INFO("ext test", "Success");
        } else {
            SC_REPORT_INFO("ext test", "Failour");
        }

        SCP_INFO() << "Uncached version empty";
        SCP_INFO(())("FMT String : Cached version default");
        SCP_INFO(SCMOD) << "UnCached version feature using SCMOD macro";
        SCP_INFO((m_my_logger)) << "Cached version using (m_my_logger)";
        SCP_INFO((D)) << "Cached version with D";
    }

    SCP_LOGGER((m_my_logger));
    SCP_LOGGER(());
    SCP_LOGGER((1), "other");
    SCP_LOGGER((D), "other", "feature.one");
};

int sc_main(int argc, char** argv) {
    cci_utils::consuming_broker broker("global_broker");
    cci_register_broker(broker);
    cci::cci_originator orig("config");
    broker.set_preset_cci_value("log_level", cci::cci_value(1), orig);
    broker.set_preset_cci_value("top.log_level", cci::cci_value(5), orig);
    broker.set_preset_cci_value("*.t3_1.log_level", cci::cci_value(5), orig);
    broker.set_preset_cci_value("feature.log_level", cci::cci_value(5), orig);

    broker.set_preset_cci_value("test4.log_level", cci::cci_value(4), orig);
    broker.set_preset_cci_value("thing1.log_level", cci::cci_value(5), orig);

    std::string logfile = "/tmp/scp_smoke_report_test." +
                          std::to_string(getpid());
    scp::init_logging(scp::LogConfig()
                          .logLevel(scp::log::DEBUG) // set log level to debug
                          .msgTypeFieldWidth(20)
                          .fileInfoFrom(5)
                          .logAsync(false)
                          .printSimTime(false));
    SCP_INFO() << "Constructing design";
    test toptest("top");
    test1 t1("t1");

    SCP_INFO() << "Starting simulation";
    sc_core::sc_start();
    SCP_WARN() << "Ending simulation";

    return 0;
}
