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

#include <scp/report.h>
#include <scp/report_cci_setter.h>
#include <systemc>
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
    SC_CTOR (test2) : t31("t3_1"), t32("t3_2"), t4("t4")
        {
            SCP_INFO(()) << "  T2 Logger()";
            SCP_WARN(()) << "  T2 Logger()";
        }
    SCP_LOGGER();
    test3 t31, t32;
    test4 t4;
};

SC_MODULE (test1) {
    SC_CTOR (test1) : t2("t2")
        {
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
    outside_class()
    {
        SCP_INFO(())("constructor");
        SCP_WARN(())("constructor");
    }
};

SC_MODULE (test) {
    outside_class oc;
    SC_CTOR (test) {
        SCP_DEBUG(SCMOD) << "First part";

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

int sc_main(int argc, char** argv)
{
    cci_utils::consuming_broker broker("global_broker");
    cci_register_broker(broker);
    cci::cci_originator orig("config");
    broker.set_preset_cci_value("log_level", cci::cci_value(1), orig);
    broker.set_preset_cci_value("top.log_level", cci::cci_value(5), orig);
    broker.set_preset_cci_value("*.t3_1.log_level", cci::cci_value(5), orig);
    broker.set_preset_cci_value("feature.log_level", cci::cci_value(5), orig);

    broker.set_preset_cci_value("test4.log_level", cci::cci_value(4), orig);
    broker.set_preset_cci_value("thing1.log_level", cci::cci_value(5), orig);

    std::string logfile = "/tmp/scp_smoke_report_test." + std::to_string(getpid());
    scp::scp_logger_from_cci cci_logger;
    scp::init_logging(scp::LogConfig()
                          .logLevel(scp::log::DEBUG) // set log level to debug
                          .msgTypeFieldWidth(20)
                          .fileInfoFrom(5)
                          .logAsync(false)
                          .printSimTime(false)
                          .logFileName(logfile)); // make the msg type column a bit tighter
    SCP_INFO() << "Constructing design";
    test toptest("top");
    test1 t1("t1");
    for (auto n : cci_logger.get_logging_parameters()) {
        SCP_INFO()("{}", n);
    }
    SCP_INFO() << "Starting simulation";
    sc_core::sc_start();
    SCP_WARN() << "Ending simulation";

#ifdef FMT_SHARED
    std::string fmtstr = "FMT String : Cached version default";
#else
    std::string fmtstr = "Please add FMT library for FMT support.";
#endif

    std::string expected =
        R"([    info] [                0 s ]SystemC             : Constructing design
[    info] [                0 s ]out.class           : constructor
[ warning] [                0 s ]out.class           : constructor
[   debug] [                0 s ]top                 : First part
[    info] [                0 s ]SystemC             : Uncached version empty
[    info] [                0 s ]top                 : FMT String : Cached version default
[    info] [                0 s ]top                 : UnCached version feature using SCMOD macro
[    info] [                0 s ]top                 : Cached version using (m_my_logger)
[    info] [                0 s ]top                 : Cached version with D
[    info] [                0 s ]t1.t2.t3_1          :  .  T3 D Logger "other" "feature.one"
[ warning] [                0 s ]t1.t2.t3_1          :  .  T3 D Logger "other" "feature.one"
[    info] [                0 s ]t1.t2.t3_1          :  .  T3 Logger ()
[ warning] [                0 s ]t1.t2.t3_1          :  .  T3 Logger ()
[    info] [                0 s ]t1.t2.t3_2          :  .  T3 D Logger "other" "feature.one"
[ warning] [                0 s ]t1.t2.t3_2          :  .  T3 D Logger "other" "feature.one"
[ warning] [                0 s ]t1.t2.t3_2          :  .  T3 Logger ()
[    info] [                0 s ]t1.t2.t4            :  .   T4 Logger() 1
[ warning] [                0 s ]t1.t2.t4            :  .   T4 Logger() 1
[    info] [                0 s ]t1.t2.t4            :  .   T4 Logger() 2
[ warning] [                0 s ]t1.t2.t4            :  .   T4 Logger() 2
[ warning] [                0 s ]t1.t2               :   T2 Logger()
[ warning] [                0 s ]My.Name             :  T1 My.Name typed log
[ warning] [                0 s ]t1                  :  T1 Logger()
[    info] [                0 s ]t1                  : Thing1?
[ warning] [                0 s ]t1                  : Thing1?
[ warning] [                0 s ]t1                  : Thing2?
[    info] [                0 s ]SystemC             : .log_level
[    info] [                0 s ]SystemC             : else.log_level
[    info] [                0 s ]SystemC             : feature.log_level
[    info] [                0 s ]SystemC             : feature.one.log_level
[    info] [                0 s ]SystemC             : other.log_level
[    info] [                0 s ]SystemC             : out.class.log_level
[    info] [                0 s ]SystemC             : out.class.out.class.log_level
[    info] [                0 s ]SystemC             : out.class.out.log_level
[    info] [                0 s ]SystemC             : out.class.outside_class.log_level
[    info] [                0 s ]SystemC             : out.class.thing1.log_level
[    info] [                0 s ]SystemC             : out.log_level
[    info] [                0 s ]SystemC             : out.out.class.log_level
[    info] [                0 s ]SystemC             : out.out.log_level
[    info] [                0 s ]SystemC             : out.outside_class.log_level
[    info] [                0 s ]SystemC             : out.thing1.log_level
[    info] [                0 s ]SystemC             : outside_class.log_level
[    info] [                0 s ]SystemC             : some.log_level
[    info] [                0 s ]SystemC             : something.log_level
[    info] [                0 s ]SystemC             : t1.else.log_level
[    info] [                0 s ]SystemC             : t1.feature.log_level
[    info] [                0 s ]SystemC             : t1.feature.one.log_level
[    info] [                0 s ]SystemC             : t1.log_level
[    info] [                0 s ]SystemC             : t1.other.log_level
[    info] [                0 s ]SystemC             : t1.some.log_level
[    info] [                0 s ]SystemC             : t1.something.log_level
[    info] [                0 s ]SystemC             : t1.t2.feature.log_level
[    info] [                0 s ]SystemC             : t1.t2.feature.one.log_level
[    info] [                0 s ]SystemC             : t1.t2.log_level
[    info] [                0 s ]SystemC             : t1.t2.other.log_level
[    info] [                0 s ]SystemC             : t1.t2.t3_1.feature.log_level
[    info] [                0 s ]SystemC             : t1.t2.t3_1.feature.one.log_level
[    info] [                0 s ]SystemC             : t1.t2.t3_1.log_level
[    info] [                0 s ]SystemC             : t1.t2.t3_1.other.log_level
[    info] [                0 s ]SystemC             : t1.t2.t3_1.test3.log_level
[    info] [                0 s ]SystemC             : t1.t2.t3_2.feature.log_level
[    info] [                0 s ]SystemC             : t1.t2.t3_2.feature.one.log_level
[    info] [                0 s ]SystemC             : t1.t2.t3_2.log_level
[    info] [                0 s ]SystemC             : t1.t2.t3_2.other.log_level
[    info] [                0 s ]SystemC             : t1.t2.t3_2.test3.log_level
[    info] [                0 s ]SystemC             : t1.t2.t4.log_level
[    info] [                0 s ]SystemC             : t1.t2.t4.test4.log_level
[    info] [                0 s ]SystemC             : t1.t2.test2.log_level
[    info] [                0 s ]SystemC             : t1.t2.test3.log_level
[    info] [                0 s ]SystemC             : t1.t2.test4.log_level
[    info] [                0 s ]SystemC             : t1.test1.log_level
[    info] [                0 s ]SystemC             : t1.test2.log_level
[    info] [                0 s ]SystemC             : t1.test3.log_level
[    info] [                0 s ]SystemC             : t1.test4.log_level
[    info] [                0 s ]SystemC             : t1.thing1.log_level
[    info] [                0 s ]SystemC             : t1.thing2.log_level
[    info] [                0 s ]SystemC             : test.log_level
[    info] [                0 s ]SystemC             : test1.log_level
[    info] [                0 s ]SystemC             : test2.log_level
[    info] [                0 s ]SystemC             : test3.log_level
[    info] [                0 s ]SystemC             : test4.log_level
[    info] [                0 s ]SystemC             : thing1.log_level
[    info] [                0 s ]SystemC             : thing2.log_level
[    info] [                0 s ]SystemC             : top..log_level
[    info] [                0 s ]SystemC             : top.feature.log_level
[    info] [                0 s ]SystemC             : top.feature.one.log_level
[    info] [                0 s ]SystemC             : top.log_level
[    info] [                0 s ]SystemC             : top.other.log_level
[    info] [                0 s ]SystemC             : top.test.log_level
[    info] [                0 s ]SystemC             : Starting simulation
[ warning] [                0 s ]SystemC             : Ending simulation
)";

    std::ifstream lf(logfile);
    std::string out((std::istreambuf_iterator<char>(lf)), std::istreambuf_iterator<char>());

    std::cout << "out file\n" << out << "\n";
    std::cout << "expected\n" << expected << "\n";
    std::cout << "Number of difference: " << out.compare(expected) << "\n";

    std::remove(logfile.c_str());
    return out.compare(expected);
}
