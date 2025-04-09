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
 * sc_report.cpp
 *
 *  Created on: 19.09.2017
 *      Author: eyck@minres.com
 *
 * THIS FILE IS INTENDED TO BE UP-STREAMED
 */
#include <systemc>
#ifndef SC_HAS_SC_LOG
#include <scp/logger.h>
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
std::unordered_map<uint64_t, sc_log::log_levels> lut;
#else
thread_local std::unordered_map<uint64_t, sc_log::log_levels> lut;
#endif

// BKDR hash algorithm
auto char_hash(char const* str) -> uint64_t
{
    constexpr unsigned int seed = 131; // 31  131 1313 13131131313 etc//
    uint64_t hash = 0;
    while (*str) {
        hash = (hash * seed) + (*str);
        str++;
    }
    return hash;
}
} // namespace

sc_log::log_levels sc_log::sc_log_logger_cache::get_log_verbosity_cached(const char* scname, const char* tname = "")
{
    if (level != sc_log::log_levels::UNSET) {
        return level;
    }

    if (!scname && features.size()) scname = features[0].c_str();
    if (!scname) scname = "";

    type = std::string(scname);

    // find the first suitable registered sc_object that can handle the lookup
    for (auto o : sc_core::sc_get_top_level_objects()) {
        auto* h = dynamic_cast<sc_log::sc_log_global_logger_handler*>(o);
        if (h) {
            auto& h_ref = *h;
            return h_ref(*this, scname, tname);
        }
    }

    return level = static_cast<sc_log::log_levels>(::sc_core::sc_report_handler::get_verbosity_level());
}

auto sc_log::get_log_verbosity(char const* str) -> sc_log::log_levels
{
    auto k = char_hash(str);
    auto it = lut.find(k);
    if (it != lut.end()) return it->second;

    sc_log::sc_log_logger_cache tmp;
    lut[k] = tmp.get_log_verbosity_cached(str);
    return lut[k];
}
#endif  // SC_HAS_SC_LOG