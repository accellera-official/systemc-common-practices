/*******************************************************************************
 * Copyright 2016-2022 MINRES Technologies GmbH
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
 *
 * THIS FILE IS INTENDED TO BE UP-STREAMED
 */
#ifndef _SCP_REPORT_H_
#define _SCP_REPORT_H_

#include <cstring>
#include <iostream>
#include <sstream>
#include <array>
#include <map>
#include <vector>

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#endif

#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_time.h>
#include <sysc/utils/sc_report.h>

#if defined(_MSC_VER) && defined(ERROR)
#undef ERROR
#endif

/* In SystemC there are 2 'scales' the severity which is INFO, WARNING, ERROR and FATAL.
 * And Verbosity which is a sub-division of INFO (NONE, LOW, MEDIUM, HIGH, FULL and DEBUG)
 *
 * Here we add Loging levels:
 *   CRITICAL, WARN,      INFO,      TRACE and    FULL.
 * Which correspond to the Verbosity levels
 *   SC_NONE, SC_LOW,     SC_MEDIUM, SC_HIGH and  SC_FULL
 */

namespace sc_core {
const sc_core::sc_verbosity SC_UNSET = (sc_core::sc_verbosity)INT_MAX;
}

//! the name of the CCI property to attach to modules to control logging of
//! this module
#define SCP_LOG_LEVEL_PARAM_NAME "log_level"

// must be global for macro to work.
static const char* _SCP_FMT_EMPTY_STR = "";

/** \ingroup scp-report
 *  @{
 */
/**@{*/
//! @brief reporting  utilities
namespace scp {

/************************
 * Provide a set of names and conversions that are suitable for logging levels based on
 * SystemC "verbosity's"
 ************************/

enum class log {
    NONE = sc_core::SC_NONE,
    CRITICAL = sc_core::SC_NONE,
    WARN = sc_core::SC_LOW,
    INFO = sc_core::SC_MEDIUM,
    TRACE = sc_core::SC_HIGH,
    FULL = sc_core::SC_DEBUG,

    DEBUG = sc_core::SC_HIGH,
    DBGTRACE = sc_core::SC_DEBUG // Only for backward compatibility
};
inline std::map<log, std::string> log_map()
{
    static std::map<log, std::string> m = {
        { log::NONE, "NONE" }, { log::CRITICAL, "CRITICAL" }, { log::WARN, "WARN" },
        { log::INFO, "INFO" }, { log::TRACE, "TRACE" },       { log::FULL, "FULL" }
    };
    return m;
}

/**
 * @fn log as_log(sc_verbosity)
 * @brief safely convert an sc_verbosity (integer) into a log level
 *
 * @param logLevel the logging level
 * @return the log level
 */
inline log as_log(sc_core::sc_verbosity logLevel)
{
    auto m = log_map();
    for (auto l : m) {
        if (logLevel <= static_cast<int>(l.first)) {
            return l.first;
        }
    }
    return log::FULL;
}

/**
 * @fn log as_log(std::string)
 * @brief safely convert a string into a log level
 *
 * @param logName the string name for the log level
 * @return the log level
 */
inline log as_log(std::string logName)
{
    auto m = log_map();
    for (auto l : m) {
        if (logName == l.second) return l.first;
    }
    sc_assert(false);
}
/**
 * @fn std::istream& operator >>(std::istream&, log&)
 * @brief read a log level from input stream e.g. used by boost::lexical_cast
 *
 * @param is input stream holding the string representation
 * @param val the value holding the resulting value
 * @return the input stream
 */
inline std::istream& operator>>(std::istream& is, log& val)
{
    std::string buf;
    is >> buf;
    val = as_log(buf);
    return is;
}
/**
 * @fn std::ostream& operator <<(std::ostream&, const log&)
 * @brief output the textual representation of the log level
 *
 * @param os output stream
 * @param val logging level
 * @return reference to the stream for chaining
 */
inline std::ostream& operator<<(std::ostream& os, log const& val)
{
    auto m = log_map();
    os << m[val];
    return os;
}

/******************/

/**
 * @brief cached logging information used in the (logger) form.
 *
 */
struct scp_logger_cache {
    sc_core::sc_verbosity level = sc_core::SC_UNSET;
    std::string type;
    std::vector<std::string> features;

    /**
     * @brief Initialize the verbosity cache and/or return the cached value.
     *
     * @return sc_core::sc_verbosity
     */
    sc_core::sc_verbosity get_log_verbosity_cached(const char*, const char*);
};

struct scp_global_logger_handler : sc_core::sc_object {
    virtual sc_core::sc_verbosity operator()(struct scp_logger_cache& logger, const char* scname,
                                             const char* tname) const = 0;
};

inline sc_core::sc_verbosity get_log_verbosity()
{
    return static_cast<sc_core::sc_verbosity>(::sc_core::sc_report_handler::get_verbosity_level());
}
/**
 * @fn sc_core::sc_verbosity get_log_verbosity(const char*)
 * @brief get the scope-based verbosity level
 *
 * The function returns a scope specific verbosity level if defined (e.g. by
 * using a CCI param named "log_level"). Otherwise the global verbosity level
 * is being returned
 *
 * @param t the SystemC hierarchy scope name
 * @return the verbosity level
 */
sc_core::sc_verbosity get_log_verbosity(char const* t);
/**
 * @fn sc_core::sc_verbosity get_log_verbosity(const char*)
 * @brief get the scope-based verbosity level
 *
 * The function returns a scope specific verbosity level if defined (e.g. by
 * using a CCI param named "log_level"). Otherwise the global verbosity level
 * is being returned
 *
 * @param t the SystemC hierarchy scope name
 * @return the verbosity level
 */
inline sc_core::sc_verbosity get_log_verbosity(std::string const& t) { return get_log_verbosity(t.c_str()); }

/**
 * @brief Return list of logging parameters that have been used
 *
 */
std::vector<std::string> get_logging_parameters();

/**
 * @struct ScLogger
 * @brief the logger class
 *
 * The ScLogger creates a RTTI based output stream to be used similar to
 * std::cout
 *
 * @tparam SEVERITY
 */
template <sc_core::sc_severity SEVERITY, bool WITH_ACTIONS = false>
struct ScLogger {
    /**
     * @fn  ScLogger(const char*, int, int=sc_core::SC_MEDIUM)
     * @brief
     *
     * @param file where the log entry originates
     * @param line number where the log entry originates
     * @param verbosity the log level
     */
    ScLogger(const char* file, int line, int verbosity = sc_core::SC_MEDIUM)
        : t(nullptr), file(file), line(line), level(verbosity)
    {
    }

    ScLogger() = delete;

    ScLogger(const ScLogger&) = delete;

    ScLogger(ScLogger&&) = delete;

    ScLogger& operator=(const ScLogger&) = delete;

    ScLogger& operator=(ScLogger&&) = delete;
    /**
     * @fn  ~ScLogger()
     * @brief the destructor generating the SystemC report
     *
     * NB a destructor should not throw an exception, here we attempt to prevent the sc_report_handler from throwing
     * The ScLogging interface is _ONLY_ for logging, simulation control should happen in user code.
     */
    virtual ~ScLogger() noexcept(true)
    {
        auto old = sc_core::sc_report_handler::set_actions(SEVERITY);
        if (WITH_ACTIONS == false) {
            sc_core::sc_report_handler::set_actions(
                SEVERITY, old & ~(sc_core::SC_THROW | sc_core::SC_INTERRUPT | sc_core::SC_STOP | sc_core::SC_ABORT));
        }
        ::sc_core::sc_report_handler::report(SEVERITY, t ? t : "SystemC", os.str().c_str(), level, file, line);
        sc_core::sc_report_handler::set_actions(SEVERITY, old);
    }
    /**
     * @fn ScLogger& type()
     * @brief reset the category of the log entry
     *
     * @return reference to self for chaining
     */
    inline ScLogger& type()
    {
        this->t = nullptr;
        return *this;
    }
    /**
     * @fn ScLogger& type(const char*)
     * @brief set the category of the log entry
     *
     * @param t type of th elog entry
     * @return reference to self for chaining
     */
    inline ScLogger& type(char const* t)
    {
        this->t = const_cast<char*>(t);
        return *this;
    }
    /**
     * @fn ScLogger& type(std::string const&)
     * @brief set the category of the log entry
     *
     * @param t type of th elog entry
     * @return reference to self for chaining
     */
    inline ScLogger& type(std::string const& t)
    {
        this->t = const_cast<char*>(t.c_str());
        return *this;
    }
    /**
     * @fn std::ostream& get()
     * @brief  get the underlying ostringstream
     *
     * @return the output stream collecting the log message
     */
    inline std::ostream& get() { return os; };

protected:
    std::ostringstream os{};
    char* t{ nullptr };
    const char* file;
    const int line;
    const int level;
};

/**
 * logging macros
 */

/**
 * Boilerplate convenience macros
 */
#define CAT(a, ...)           PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a##__VA_ARGS__

#define IIF(c)        PRIMITIVE_CAT(IIF_, c)
#define IIF_0(t, ...) __VA_ARGS__
#define IIF_1(t, ...) t

#define CHECK_N(x, n, ...) n
#define CHECK(...)         CHECK_N(__VA_ARGS__, 0, )
#define PROBE(x)           x, 1,

#define EXPAND(...) __VA_ARGS__

#define FIRST_ARG(f, ...) f
#define POP_ARG(f, ...)   __VA_ARGS__

#define STR_HELPER(x) #x
#define STR(x)        STR_HELPER(x)

#define IS_PAREN(x)         CHECK(IS_PAREN_PROBE x)
#define IS_PAREN_PROBE(...) PROBE(~)
/********/

/* default logger cache name */
#define SCP_REPORTER_NAME(x) CAT(_m_scp_log_level_cache_, x)

/* User interface macros */
#define SCMOD this->sc_core::sc_module::name()
#define SCP_REPORTER(...)                                            \
    scp::scp_logger_cache IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))(     \
        SCP_REPORTER_NAME(EXPAND(FIRST_ARG FIRST_ARG(__VA_ARGS__))), \
        SCP_REPORTER_NAME()) = { sc_core::SC_UNSET,                  \
                                 "",                                 \
                                 { IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))(POP_ARG(__VA_ARGS__), ##__VA_ARGS__) } }

#define SCP_REPORTER_VECTOR(NAME) std::vector<scp::scp_logger_cache> SCP_REPORTER_NAME(NAME)
#define SCP_REPORTER_VECTOR_PUSH_BACK(NAME, ...) \
    SCP_REPORTER_NAME(NAME).push_back({ sc_core::SC_UNSET, "", { __VA_ARGS__ } });

class call_sc_name_fn
{
    template <class T>
    static auto test(T* p) -> decltype(p->sc_core::sc_module::name(), std::true_type());
    template <class T>
    static auto test(...) -> decltype(std::false_type());

    template <class T>
    static constexpr bool has_method = decltype(test<T>(nullptr))::value;

public:
    // define a function IF the method exists
    template <class TYPE>
    auto operator()(TYPE* p) const -> std::enable_if_t<has_method<TYPE>, const char*>
    {
        return p->sc_core::sc_module::name();
    }

    // define a function IF NOT the method exists
    template <class TYPE>
    auto operator()(TYPE* p) const -> std::enable_if_t<!has_method<TYPE>, const char*>
    {
        return nullptr;
    }
};

// critical thing is that the initial if 'fails' as soon as possible - if it is
// going to pass, we have all the time we want, as we will be logging anyway
// This HAS to be done as a macro, because the first argument may be a string
// or a cache'd level

/*** Helper macros for SCP_ report macros ****/
#define SCP_VBSTY_CHECK_CACHED(lvl, features, cached, ...) \
    (cached.level >= lvl) &&                               \
        (cached.get_log_verbosity_cached(scp::call_sc_name_fn()(this), typeid(*this).name()) >= lvl)

#define SCP_VBSTY_CHECK_UNCACHED(lvl, ...) (::scp::get_log_verbosity(__VA_ARGS__) >= lvl)

#define SCP_VBSTY_CHECK(lvl, ...)                                                                                      \
    IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))                                                                              \
    (SCP_VBSTY_CHECK_CACHED(lvl, FIRST_ARG(__VA_ARGS__), SCP_REPORTER_NAME(EXPAND(FIRST_ARG FIRST_ARG(__VA_ARGS__)))), \
     SCP_VBSTY_CHECK_UNCACHED(lvl, ##__VA_ARGS__))

#define SCP_GET_FEATURES(...)                                                                                    \
    IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))                                                                        \
    (FIRST_ARG EXPAND((POP_ARG(__VA_ARGS__, SCP_REPORTER_NAME(EXPAND(FIRST_ARG FIRST_ARG(__VA_ARGS__))).type))), \
     __VA_ARGS__)

#ifdef FMT_SHARED
#define _SCP_FMT_EMPTY_STR(...) fmt::format(__VA_ARGS__)
#else
#define _SCP_FMT_EMPTY_STR(...) "Please add FMT library for FMT support."
#endif

#define SCP_MSG(lvl, ...)                                                                                         \
    ::scp::ScLogger<::sc_core::SC_INFO, false>(__FILE__, __LINE__, lvl).type(SCP_GET_FEATURES(__VA_ARGS__)).get() \
        << _SCP_FMT_EMPTY_STR
/*** End HELPER Macros *******/

#define SCP_LOG(lvl, ...) \
    if (SCP_VBSTY_CHECK(lvl, ##__VA_ARGS__)) SCP_MSG(lvl, __VA_ARGS__)

#define SCCRITICAL(...) SCP_LOG(log::CRITICAL, ##__VA_ARGS__)
#define SCWARN(...)     SCP_LOG(log::WARN, ##__VA_ARGS__)
#define SCINFO(...)     SCP_LOG(log::INFO, ##__VA_ARGS__)
#define SCTRACE(...)    SCP_LOG(log::TRACE, ##__VA_ARGS__)
#define SCFULL(...)     SCP_LOG(log::FULL, ##__VA_ARGS__)

/* Only for backwards compatibility with SCP library */
#define SCP_FATAL(...)                                                                                            \
    ::scp::ScLogger<::sc_core::SC_FATAL, true>(__FILE__, __LINE__, sc_core::SC_NONE).type(SCP_GET_FEATURES(__VA_ARGS__)).get() \
        << _SCP_FMT_EMPTY_STR
#define SCP_ERR(...)                                                                                              \
    ::scp::ScLogger<::sc_core::SC_ERROR, true>(__FILE__, __LINE__, sc_core::SC_NONE).type(SCP_GET_FEATURES(__VA_ARGS__)).get() \
        << _SCP_FMT_EMPTY_STR
#define SCP_WARN(...)     SCP_LOG(sc_core::SC_LOW, ##__VA_ARGS__)
#define SCP_INFO(...)     SCP_LOG(sc_core::SC_MEDIUM, ##__VA_ARGS__)
#define SCP_DEBUG(...)    SCP_LOG(sc_core::SC_HIGH, ##__VA_ARGS__)
#define SCP_TRACE(...)    SCP_LOG(sc_core::SC_FULL, ##__VA_ARGS__)
#define SCP_TRACEALL(...) SCP_LOG(sc_core::SC_DEBUG, ##__VA_ARGS__)

} // namespace scp
/** @} */ // end of scp-report
#endif    /* _SCP_REPORT_H_ */
