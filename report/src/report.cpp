/*******************************************************************************
 * Copyright 2017-2022 MINRES Technologies GmbH
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
/*
 * report.cpp
 *
 *  Created on: 19.09.2017
 *      Author: eyck@minres.com
 */

#include <scp/logger.h>
#include <array>
#include <numeric>
#include <systemc>
#ifdef HAS_CCI
#include <cci_configuration>
#endif
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <unordered_map>
#if defined(__GNUC__) || defined(__clang__)
#define likely(x)   __builtin_expect(x, 1)
#define unlikely(x) __builtin_expect(x, 0)
#else
#define likely(x)   x
#define unlikely(x) x
#endif

#ifdef ERROR
#undef ERROR
#endif

namespace {
// Making this thread_local could cause thread copies of the same cache
// entries, but more likely naming will be thread local too, and this avoids
// races in the unordered_map

#ifdef DISABLE_REPORT_THREAD_LOCAL
std::unordered_map<uint64_t, sc_core::sc_verbosity> lut;
#else
thread_local std::unordered_map<uint64_t, sc_core::sc_verbosity> lut;
#endif

#ifdef HAS_CCI
cci::cci_originator scp_global_originator("scp_reporting_global");
#endif

std::set<std::string> logging_parameters;

// BKDR hash algorithm
auto char_hash(char const* str) -> uint64_t {
    constexpr unsigned int seed = 131; // 31  131 1313 13131131313 etc//
    uint64_t hash = 0;
    while (*str) {
        hash = (hash * seed) + (*str);
        str++;
    }
    return hash;
}
} // namespace

static const std::array<sc_core::sc_severity, 8> severity = {
    sc_core::SC_FATAL,   // scp::log::NONE
    sc_core::SC_FATAL,   // scp::log::FATAL
    sc_core::SC_ERROR,   // scp::log::ERROR
    sc_core::SC_WARNING, // scp::log::WARNING
    sc_core::SC_INFO,    // scp::log::INFO
    sc_core::SC_INFO,    // scp::log::DEBUG
    sc_core::SC_INFO,    // scp::log::TRACE
    sc_core::SC_INFO     // scp::log::TRACEALL
};
static const std::array<sc_core::sc_verbosity, 8> verbosity = {
    sc_core::SC_NONE,   // scp::log::NONE
    sc_core::SC_LOW,    // scp::log::FATAL
    sc_core::SC_LOW,    // scp::log::ERROR
    sc_core::SC_LOW,    // scp::log::WARNING
    sc_core::SC_MEDIUM, // scp::log::INFO
    sc_core::SC_HIGH,   // scp::log::DEBUG
    sc_core::SC_FULL,   // scp::log::TRACE
    sc_core::SC_DEBUG   // scp::log::TRACEALL
};

std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> result;
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, '.')) {
        result.push_back(item);
    }
    return result;
}

std::string join(std::vector<std::string> vec) {
    if (vec.empty())
        return "";
    return std::accumulate(
        vec.begin(), vec.end(), std::string(),
        [](const std::string& a, const std::string& b) -> std::string {
            return a + (a.length() > 0 ? "." : "") + b;
        });
}

std::vector<std::string> scp::get_logging_parameters() {
    return std::vector<std::string>(logging_parameters.begin(),
                                    logging_parameters.end());
}

sc_core::sc_verbosity cci_lookup(cci::cci_broker_handle broker,
                                 std::string name) {
    auto param_name = (name.empty()) ? SCP_LOG_LEVEL_PARAM_NAME
                                     : name + "." SCP_LOG_LEVEL_PARAM_NAME;
    auto h = broker.get_param_handle(param_name);
    if (h.is_valid()) {
        return verbosity.at(std::min<unsigned>(h.get_cci_value().get_int(),
                                               verbosity.size() - 1));
    } else {
        auto val = broker.get_preset_cci_value(param_name);

        if (val.is_int()) {
            broker.lock_preset_value(param_name);
            return verbosity.at(
                std::min<unsigned>(val.get_int(), verbosity.size() - 1));
        }
    }
    return sc_core::SC_UNSET;
}

#ifdef __GNUG__
std::string demangle(const char* name) {
    int status = -4; // some arbitrary value to eliminate the compiler
                     // warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void (*)(void*)> res{
        abi::__cxa_demangle(name, NULL, NULL, &status), std::free
    };

    return (status == 0) ? res.get() : name;
}
#else
// does nothing if not GNUG
std::string demangle(const char* name) {
    return name;
}
#endif

void insert(std::multimap<int, std::string, std::greater<int>>& map,
            std::string s, bool interesting) {
    int n = std::count(s.begin(), s.end(), '.');
    map.insert(make_pair(n, s));

    if (interesting) {
        logging_parameters.insert(s + "." SCP_LOG_LEVEL_PARAM_NAME);
    }
}

sc_core::sc_verbosity scp::scp_logger_cache::get_log_verbosity_cached(
    const char* scname, const char* tname = "") {
    if (level != sc_core::SC_UNSET) {
        return level;
    }

    if (!scname && features.size())
        scname = features[0].c_str();
    if (!scname)
        scname = "";

    type = std::string(scname);

#ifdef HAS_CCI
    try {
        // we rely on there being a broker, allow this to throw if not
        auto broker = sc_core::sc_get_current_object()
                          ? cci::cci_get_broker()
                          : cci::cci_get_global_broker(scp_global_originator);

        std::multimap<int, std::string, std::greater<int>> allfeatures;

        /* initialize */
        for (auto scn = split(scname); scn.size(); scn.pop_back()) {
            for (int first = 0; first < scn.size(); first++) {
                auto f = scn.begin() + first;
                std::vector<std::string> p(f, scn.end());
                auto scn_str = ((first > 0) ? "*." : "") + join(p);

                for (auto ft : features) {
                    for (auto ftn = split(ft); ftn.size(); ftn.pop_back()) {
                        insert(allfeatures, scn_str + "." + join(ftn),
                               first == 0);
                    }
                }
                insert(allfeatures, scn_str + "." + demangle(tname),
                       first == 0);
                insert(allfeatures, scn_str, first == 0);
            }
        }
        for (auto ft : features) {
            for (auto ftn = split(ft); ftn.size(); ftn.pop_back()) {
                insert(allfeatures, join(ftn), true);
                insert(allfeatures, "*." + join(ftn), false);
            }
        }
        insert(allfeatures, demangle(tname), true);
        insert(allfeatures, "*", false);
        insert(allfeatures, "", false);

        for (std::pair<int, std::string> f : allfeatures) {
            sc_core::sc_verbosity v = cci_lookup(broker, f.second);
            if (v != sc_core::SC_UNSET) {
                level = v;
                return v;
            }
        }
    } catch (const std::exception&) {
        // If there is no global broker, revert to initialized verbosity level
    }

#endif

    return level = static_cast<sc_core::sc_verbosity>(
               ::sc_core::sc_report_handler::get_verbosity_level());
}

auto scp::get_log_verbosity(char const* str) -> sc_core::sc_verbosity {
    auto k = char_hash(str);
    auto it = lut.find(k);
    if (it != lut.end())
        return it->second;

    scp::scp_logger_cache tmp;
    lut[k] = tmp.get_log_verbosity_cached(str);
    return lut[k];
}
