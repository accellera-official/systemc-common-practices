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
#ifndef _SC_LOG_REPORT_H_
#define _SC_LOG_REPORT_H_

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

//! the name of the CCI property to attach to modules to control logging of
//! this module
#define SC_LOG_LEVEL_PARAM_NAME "log_level"

// must be global for macro to work.
static const char* _SCP_FMT_EMPTY_STR = "";

/** \ingroup sc_log-report
 *  @{
 */
/**@{*/
//! @brief reporting  utilities
namespace sc_log {

/************************
 * Provide a set of names and conversions that are suitable for logging levels based on
 * SystemC "verbosity's"
 ************************/

enum class log_levels {
    NONE = sc_core::SC_NONE,
    CRITICAL = sc_core::SC_NONE,
    WARN = sc_core::SC_LOW,
    INFO = sc_core::SC_MEDIUM,
    DEBUG = sc_core::SC_HIGH,
    TRACE = sc_core::SC_DEBUG,

    UNSET = INT_MAX
};
inline std::map<log_levels, std::string> log_map()
{
    static std::map<log_levels, std::string> m = { { log_levels::NONE, "NONE" },   { log_levels::CRITICAL, "CRITICAL" },
                                                   { log_levels::WARN, "WARN" },   { log_levels::INFO, "INFO" },
                                                   { log_levels::DEBUG, "DEBUG" }, { log_levels::TRACE, "TRACE" } };
    return m;
}

/**
 * @fn log as_log(int)
 * @brief safely convert an integer into a log level
 *
 * @param logLevel the logging level
 * @return the log level
 */
inline log_levels as_log(int logLevel)
{
    auto m = log_map();
    for (auto l : m) {
        if (logLevel <= static_cast<int>(l.first)) {
            return l.first;
        }
    }
    return log_levels::TRACE;
}

/**
 * @fn log as_log(std::string)
 * @brief safely convert a string into a log level
 *
 * @param logName the string name for the log level
 * @return the log level
 */
inline log_levels as_log(std::string logName)
{
    auto m = log_map();
    for (auto l : m) {
        if (logName <= l.second) return l.first;
    }
    return log_levels::TRACE;
}
/**
 * @fn std::istream& operator >>(std::istream&, log&)
 * @brief read a log level from input stream e.g. used by boost::lexical_cast
 *
 * @param is input stream holding the string representation
 * @param val the value holding the resulting value
 * @return the input stream
 */
inline std::istream& operator>>(std::istream& is, log_levels& val)
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
inline std::ostream& operator<<(std::ostream& os, log_levels const& val)
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
struct sc_log_logger_cache {
    log_levels level = log_levels::UNSET;
    std::string type;
    std::vector<std::string> features;

    /**
     * @brief Initialize the verbosity cache and/or return the cached value.
     *
     * @return log
     */
    log_levels get_log_verbosity_cached(const char*, const char*);
};

struct sc_log_global_logger_handler : sc_core::sc_object {
    virtual log_levels operator()(struct sc_log_logger_cache& logger, const char* scname, const char* tname) const = 0;
};

inline log_levels get_log_verbosity()
{
    return static_cast<log_levels>(::sc_core::sc_report_handler::get_verbosity_level());
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

log_levels get_log_verbosity(char const* t);
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
inline log_levels get_log_verbosity(std::string const& t) { return get_log_verbosity(t.c_str()); }

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
    ScLogger(const char* file, int line, log_levels verbosity = sc_log::log_levels::INFO)
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
        ::sc_core::sc_report_handler::report(SEVERITY, t ? t : "SystemC", os.str().c_str(),
                                             static_cast<sc_core::sc_verbosity>(level), file, line);
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
    const log_levels level;
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
#define SC_LOG_HANDLE_NAME(x) CAT(_m_sc_log_log_level_cache_, x)

/* User interface macros */
#define SCMOD this->sc_core::sc_module::name()
#define SC_LOG_HANDLE(...)                                             \
    sc_log::sc_log_logger_cache IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))( \
        SC_LOG_HANDLE_NAME(EXPAND(FIRST_ARG FIRST_ARG(__VA_ARGS__))),  \
        SC_LOG_HANDLE_NAME()) = { sc_log::log_levels::UNSET,           \
                                  "",                                  \
                                  { IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))(POP_ARG(__VA_ARGS__), ##__VA_ARGS__) } }

#define SC_LOG_HANDLE_VECTOR(NAME) std::vector<sc_log::sc_log_logger_cache> SC_LOG_HANDLE_NAME(NAME)
#define SC_LOG_HANDLE_VECTOR_PUSH_BACK(NAME, ...) \
    SC_LOG_HANDLE_NAME(NAME).push_back({ sc_log::log_levels::UNSET, "", { __VA_ARGS__ } });

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
        (cached.get_log_verbosity_cached(sc_log::call_sc_name_fn()(this), typeid(*this).name()) >= lvl)

#define SCP_VBSTY_CHECK_UNCACHED(lvl, ...) (::sc_log::get_log_verbosity(__VA_ARGS__) >= lvl)

#define SCP_VBSTY_CHECK(lvl, ...)                                                          \
    IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))                                                  \
    (SCP_VBSTY_CHECK_CACHED(lvl, FIRST_ARG(__VA_ARGS__),                                   \
                            SC_LOG_HANDLE_NAME(EXPAND(FIRST_ARG FIRST_ARG(__VA_ARGS__)))), \
     SCP_VBSTY_CHECK_UNCACHED(lvl, ##__VA_ARGS__))

#define SCP_GET_FEATURES(...)                                                                                     \
    IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))                                                                         \
    (FIRST_ARG EXPAND((POP_ARG(__VA_ARGS__, SC_LOG_HANDLE_NAME(EXPAND(FIRST_ARG FIRST_ARG(__VA_ARGS__))).type))), \
     __VA_ARGS__)

#ifdef FMT_SHARED
#define _SCP_FMT_EMPTY_STR(...) fmt::format(__VA_ARGS__)
#else
#define _SCP_FMT_EMPTY_STR(...) "Please add FMT library for FMT support."
#endif

#define SCP_MSG(lvl, ...)                                                                                            \
    ::sc_log::ScLogger<::sc_core::SC_INFO, false>(__FILE__, __LINE__, lvl).type(SCP_GET_FEATURES(__VA_ARGS__)).get() \
        << _SCP_FMT_EMPTY_STR
/*** End HELPER Macros *******/

#define SC_LOG_AT(lvl, ...) \
    if (SCP_VBSTY_CHECK(lvl, __VA_ARGS__)) SCP_MSG(lvl, __VA_ARGS__)

#define SC_CRITICAL(...) SC_LOG_AT(sc_log::log_levels::CRITICAL, _VA_ARGS__)
#define SC_WARN(...)     SC_LOG_AT(sc_log::log_levels::WARN, __VA_ARGS__)
#define SC_INFO(...)     SC_LOG_AT(sc_log::log_levels::INFO, __VA_ARGS__)
#define SC_DEBUG(...)    SC_LOG_AT(sc_log::log_levels::DEBUG, __VA_ARGS__)
#define SC_TRACE(...)    SC_LOG_AT(sc_log::log_levels::TRACE, __VA_ARGS__)

} // namespace sc_log
/** @} */ // end of sc_log-report
#endif    /* _SC_LOG_REPORT_H_ */
