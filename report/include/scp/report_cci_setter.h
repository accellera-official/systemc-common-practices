/*******************************************************************************
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
#ifndef _SCP_REPORT_CCI_SETTER_H_
#define _SCP_REPORT_CCI_SETTER_H_

#include <systemc>
#include <sc_log/sc_log.h>
#include <cci_configuration>
#include <type_traits>
#include <regex>
#include <numeric>

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#endif

#define SC_LOG_LEVEL_PARAM_NAME "log_level"

namespace scp {
static std::set<std::string> logging_parameters;

class scp_logger_from_cci : public sc_log::sc_log_global_logger_handler
{
    std::vector<std::string> split(const std::string& s) const
    {
        std::vector<std::string> result;
        std::istringstream iss(s);
        std::string item;
        while (std::getline(iss, item, '.')) {
            result.push_back(item);
        }
        return result;
    }

    std::string join(std::vector<std::string> vec) const
    {
        if (vec.empty()) return "";
        return std::accumulate(vec.begin(), vec.end(), std::string(),
                               [](const std::string& a, const std::string& b) -> std::string {
                                   return a + (a.length() > 0 ? "." : "") + b;
                               });
    }

    void insert(std::multimap<int, std::string, std::greater<int>>& map, std::string s, bool interesting) const
    {
        int n = std::count(s.begin(), s.end(), '.');
        map.insert(make_pair(n, s));

        if (interesting) {
            logging_parameters.insert(s + "." SC_LOG_LEVEL_PARAM_NAME);
        }
    }
    sc_log::log_levels cci_lookup(cci::cci_broker_handle broker, std::string name) const
    {
        auto param_name = (name.empty()) ? SC_LOG_LEVEL_PARAM_NAME : name + "." SC_LOG_LEVEL_PARAM_NAME;
        auto h = broker.get_param_handle(param_name);
        if (h.is_valid()) {
            return static_cast<sc_log::log_levels>(h.get_cci_value().get_int());
        } else {
            auto val = broker.get_preset_cci_value(param_name);

            if (val.is_int()) {
                broker.lock_preset_value(param_name);
                if (val.get_int() < 100)
                    return static_cast<sc_log::log_levels>(val.get_int() * 100); // For 'old' SCP support
                return static_cast<sc_log::log_levels>(val.get_int());
            }
            if (val.is_string()) {
                broker.lock_preset_value(param_name);
                return static_cast<sc_log::log_levels>(sc_log::as_log(val.get_string()));
            }
        }
        return sc_log::log_levels::UNSET;
    }
#ifdef __GNUG__
    std::string demangle(const char* name) const
    {
        int status = -4; // some arbitrary value to eliminate the compiler
                         // warning

        // enable c++11 by passing the flag -std=c++11 to g++
        std::unique_ptr<char, void (*)(void*)> res{ abi::__cxa_demangle(name, NULL, NULL, &status), std::free };

        return (status == 0) ? res.get() : name;
    }
#else
    // does nothing if not GNUG
    std::string demangle(const char* name) { return name; }
#endif
public:
    sc_log::log_levels operator()(struct sc_log::sc_log_logger_cache& logger, std::string_view scname,
                                  const char* tname) const
    {
        try {
            // we rely on there being a broker, allow this to throw if not
            auto broker = sc_core::sc_get_current_object()
                              ? cci::cci_get_broker()
                              : cci::cci_get_global_broker(cci::cci_originator("sc_log_reporting_global"));

            std::multimap<int, std::string, std::greater<int>> allfeatures;

            /* initialize */
            for (auto scn = split(std::string(scname)); scn.size(); scn.pop_back()) {
                for (int first = 0; first < scn.size(); first++) {
                    auto f = scn.begin() + first;
                    std::vector<std::string> p(f, scn.end());
                    auto scn_str = ((first > 0) ? "*." : "") + join(p);

                    for (auto ft : logger.features) {
                        for (auto ftn = split(ft); ftn.size(); ftn.pop_back()) {
                            insert(allfeatures, scn_str + "." + join(ftn), first == 0);
                        }
                    }
                    insert(allfeatures, scn_str + "." + demangle(tname), first == 0);
                    insert(allfeatures, scn_str, first == 0);
                }
            }
            for (auto ft : logger.features) {
                for (auto ftn = split(ft); ftn.size(); ftn.pop_back()) {
                    insert(allfeatures, join(ftn), true);
                    insert(allfeatures, "*." + join(ftn), false);
                }
            }
            insert(allfeatures, demangle(tname), true);
            insert(allfeatures, "*", false);
            insert(allfeatures, "", false);

            for (std::pair<int, std::string> f : allfeatures) {
                sc_log::log_levels v = cci_lookup(broker, f.second);
                if (v != sc_log::log_levels::UNSET) {
                    logger.level = v;
                    return v;
                }
            }
        } catch (const std::exception&) {
            // If there is no global broker, revert to initialized verbosity
            // level
        }
        return logger.level = static_cast<sc_log::log_levels>(::sc_core::sc_report_handler::get_verbosity_level());
    }
    static std::vector<std::string> get_logging_parameters()
    {
        return std::vector<std::string>(logging_parameters.begin(), logging_parameters.end());
    }
};

} // namespace scp

#endif