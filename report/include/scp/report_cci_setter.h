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

#include <scp/report.h>
#include <cci_configuration>
#include <type_traits>
#include <regex>

namespace scp {
static std::set<std::string> logging_parameters;

class scp_logger_from_cci
{
    std::vector<std::string> split(const std::string& s) const {
        std::vector<std::string> result;
        std::istringstream iss(s);
        std::string item;
        while (std::getline(iss, item, '.')) {
            result.push_back(item);
        }
        return result;
    }

    std::string join(std::vector<std::string> vec) const {
        if (vec.empty())
            return "";
        return std::accumulate(
            vec.begin(), vec.end(), std::string(),
            [](const std::string& a, const std::string& b) -> std::string {
                return a + (a.length() > 0 ? "." : "") + b;
            });
    }

    void insert(std::multimap<int, std::string, std::greater<int>>& map,
                std::string s, bool interesting) const {
        int n = std::count(s.begin(), s.end(), '.');
        map.insert(make_pair(n, s));

        if (interesting) {
            logging_parameters.insert(s + "." SCP_LOG_LEVEL_PARAM_NAME);
        }
    }
    sc_core::sc_verbosity cci_lookup(cci::cci_broker_handle broker,
                                     std::string name) const {
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
    std::string demangle(const char* name) const {
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
    std::string demangle(const char* name) { return name; }
#endif
public:
    sc_core::sc_verbosity operator()(struct scp_logger_cache& logger,
                                     const char* scname,
                                     const char* tname) const {
        try {
            // we rely on there being a broker, allow this to throw if not
            auto broker = sc_core::sc_get_current_object()
                              ? cci::cci_get_broker()
                              : cci::cci_get_global_broker(cci::cci_originator(
                                    "scp_reporting_global"));

            std::multimap<int, std::string, std::greater<int>> allfeatures;

            /* initialize */
            for (auto scn = split(scname); scn.size(); scn.pop_back()) {
                for (int first = 0; first < scn.size(); first++) {
                    auto f = scn.begin() + first;
                    std::vector<std::string> p(f, scn.end());
                    auto scn_str = ((first > 0) ? "*." : "") + join(p);

                    for (auto ft : logger.features) {
                        for (auto ftn = split(ft); ftn.size();
                             ftn.pop_back()) {
                            insert(allfeatures, scn_str + "." + join(ftn),
                                   first == 0);
                        }
                    }
                    insert(allfeatures, scn_str + "." + demangle(tname),
                           first == 0);
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
                sc_core::sc_verbosity v = cci_lookup(broker, f.second);
                if (v != sc_core::SC_UNSET) {
                    logger.level = v;
                    return v;
                }
            }
        } catch (const std::exception&) {
            // If there is no global broker, revert to initialized verbosity
            // level
        }
        return logger.level = static_cast<sc_core::sc_verbosity>(
                   ::sc_core::sc_report_handler::get_verbosity_level());
    }
    static std::vector<std::string> get_logging_parameters() {
        return std::vector<std::string>(logging_parameters.begin(),
                                        logging_parameters.end());
    }
};

#if 0

/*template <typename T, typename = int>
struct HasLogger : std::false_type { };

template <typename T>
struct HasLogger <T, decltype((void) T::SCP_LOGGER_NAME(), 0)> : std::true_type
{ };
*/

class set_logger_level
{
    template <class T>
    static auto test(T* p) -> decltype(p->SCP_LOGGER_NAME(), std::true_type());
    template <class T>
    static auto test(...) -> decltype(std::false_type());

    template <class T>
    static constexpr bool has_logger = decltype(test<T>(nullptr))::value;

public:
    // define a function IF the method exists
    template <class TYPE>
    auto operator()(TYPE* p, sc_core::sc_verbosity level) const
        -> std::enable_if_t<has_logger<TYPE>> {
        p->SCP_LOGGER_NAME().level = level;
    }

    // define a function IF NOT the method exists
    template <class TYPE>
    auto operator()(TYPE* p, sc_core::sc_verbosity level) const
        -> std::enable_if_t<!has_logger<TYPE>> {}
};

int set_log_levels(std::string path, sc_core::sc_verbosity level,
                   sc_core::sc_object* m = nullptr, bool set = false) {
    int found = 0;
    auto pathdot = path.find_first_of('.');
    auto lpath = (pathdot != std::string::npos) ? path.substr(pathdot) : path;

    if (m) {
        std::string mname = std::string(m->name());
        auto namedot = mname.find_last_of('.');
        auto lname = (namedot != std::string::npos) ? mname.substr(namedot)
                                                    : mname;

        if (!((lpath == "*") || (lpath == lname))) {
            // no match here
            return found;
        }
    }
    if (path.substr(pathdot) == SCP_LOG_LEVEL_PARAM_NAME) {
        // Found it
        set = true;
    }

    if (set) {
        found++;
        scp::set_logger_level()(m, level);
    }
    std::vector<sc_core::sc_object*> children;
    if (m) {
        children = m->get_child_objects();
    } else {
        children = sc_core::sc_get_top_level_objects();
    }

    for (auto c : children) {
        if (lpath == "*") {
            found += set_log_levels(path, level, c, set);
        }
        if (pathdot != std::string::npos) {
            found += set_log_levels(path.substr(pathdot), level, c, set);
        }
    }
    return found;
};

void cci_enable_logging(cci::cci_broker_handle broker) {
    std::string pname = std::string(".") + SCP_LOG_LEVEL_PARAM_NAME;
    int plen = std::string(SCP_LOG_LEVEL_PARAM_NAME).length();
    std::vector<cci::cci_name_value_pair> logging_paths;
    for (auto path : broker.get_unconsumed_preset_values(
             [&plen](const std::pair<std::string, cci::cci_value>& iv) {
                 if (iv.first.length() >= plen) {
                     return (0 == iv.first.compare(iv.first.length() - plen,
                                                   plen,
                                                   SCP_LOG_LEVEL_PARAM_NAME));
                 } else {
                     return false;
                 }
             })) {
        logging_paths.push_back(path);
    }

    std::sort(logging_paths.begin(), logging_paths.end(),
              [&](cci::cci_name_value_pair& s1, cci::cci_name_value_pair& s2) {
                  return std::count(s1.first.begin(), s1.first.end(), '.') >
                         std::count(s2.first.begin(), s2.first.end(), '.');
              });
    for (auto path : logging_paths) {
        int found = set_log_levels(
            path.first, (sc_core::sc_verbosity)(path.second.get_int()));
        if (found == 0) {
            SCP_WARN()("No logger named {} found", path.first);
        } else {
            SCP_INFO()("{} loggers enabled from name {}", found, path.first);
        }
    }
}
#endif

} // namespace scp

#endif