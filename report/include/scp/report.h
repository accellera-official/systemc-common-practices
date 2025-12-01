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

#ifndef _SCP_REPORT_H_
#define _SCP_REPORT_H_

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <array>
#include <numeric>
#include <vector>
#include <numeric>
#include <functional>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_time.h>
#include <sysc/utils/sc_report.h>

#ifdef FMT_SHARED
#include <fmt/format.h>
#endif

// Due to ERROR being a macro defined on Windows platforms,
// we need to #undef it to avoid a collision with log:ERROR.
#undef ERROR

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
//! \brief array holding string representations of log levels
static std::array<const char* const, 8> buffer = {
    { "NONE", "FATAL", "ERROR", "WARNING", "INFO", "DEBUG", "TRACE",
      "TRACEALL" }
};
//! \brief enum defining the log levels
enum class log {
    NONE,
    FATAL,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
    TRACE,
    TRACEALL,
    DBGTRACE = TRACEALL
};

/**
 * @fn log as_log(int)
 * @brief safely convert an integer into a log level
 *
 * @param logLevel the logging level
 * @return the log level
 */
inline log as_log(int logLevel) {
    assert(logLevel >= static_cast<int>(log::NONE) &&
           logLevel <= static_cast<int>(log::TRACEALL));
    std::array<const log, 8> m = { { log::NONE, log::FATAL, log::ERROR,
                                     log::WARNING, log::INFO, log::DEBUG,
                                     log::TRACE, log::TRACEALL } };
    return m[logLevel];
}
/**
 * @fn std::istream& operator >>(std::istream&, log&)
 * @brief read a log level from input stream e.g. used by boost::lexical_cast
 *
 * @param is input stream holding the string representation
 * @param val the value holding the resulting value
 * @return the input stream
 */
inline std::istream& operator>>(std::istream& is, log& val) {
    std::string buf;
    is >> buf;
    for (auto i = 0U; i <= static_cast<unsigned>(log::TRACEALL); ++i) {
        if (std::strcmp(buf.c_str(), buffer[i]) == 0) {
            val = as_log(i);
            return is;
        }
    }
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
inline std::ostream& operator<<(std::ostream& os, log const& val) {
    os << buffer[static_cast<unsigned>(val)];
    return os;
}
/**
 * @fn void init_logging(log=log::WARNING, unsigned=24, bool=false)
 * @brief initializes the SystemC logging system with a particular logging
 * level
 *
 * @param level the log level
 * @param type_field_width the with of the type field in the output
 * @param print_time whether to print the system time stamp
 */
void init_logging(log level = log::WARNING, unsigned type_field_width = 24,
                  bool print_time = false);
/**
 * @fn void init_logging(log=log::WARNING, unsigned=24, bool=false)
 * @brief initializes the SystemC logging system with a particular logging
 * level
 *
 * @param level the log level
 * @param type_field_width the with of the type field in the output
 * @param print_time whether to print the system time stamp
 */
void reinit_logging(log level = log::WARNING);
/**
 * @struct LogConfig
 * @brief the configuration class for the logging setup
 *
 * using this class allows to configure the logging output in many aspects. The
 * class follows the builder pattern.
 */
struct LogConfig {
    log level{ log::WARNING };
    unsigned msg_type_field_width{ 24 };
    bool print_sys_time{ false };
    bool print_sim_time{ true };
    bool print_delta{ false };
    bool print_severity{ true };
    bool colored_output{ true };
    std::string log_file_name{ "" };
    std::string log_filter_regex{ "" };
    bool log_async{ true };
    bool report_only_first_error{ false };
    int file_info_from{ sc_core::SC_INFO };

    //! set the logging level
    LogConfig& logLevel(log);
    //! define the width of the message field, 0 to disable,
    //! std::numeric_limits<unsigned>::max() for arbitrary width
    LogConfig& msgTypeFieldWidth(unsigned);
    //! enable/disable printing of system time
    LogConfig& printSysTime(bool = true);
    //! enable/disable printing of simulation time
    LogConfig& printSimTime(bool = true);
    //! enable/disable printing delta cycles
    LogConfig& printDelta(bool = true);
    //! enable/disable printing of severity level
    LogConfig& printSeverity(bool = true);
    //! enable/disable colored output
    LogConfig& coloredOutput(bool = true);
    //! set the file name for the log output file
    LogConfig& logFileName(std::string&&);
    //! set the file name for the log output file
    LogConfig& logFileName(const std::string&);
    //! set the regular expression to filter the output
    LogConfig& logFilterRegex(std::string&&);
    //! set the regular expression to filter the output
    LogConfig& logFilterRegex(const std::string&);
    //! enable/disable asynchronous output (write to file in separate thread
    LogConfig& logAsync(bool = true);
    //! disable the printing of the file name from this level upwards.
    LogConfig& fileInfoFrom(int);
    //! disable/enable the supression of all error messages after the first
    LogConfig& reportOnlyFirstError(bool = true);
};

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

/**
 * @fn void init_logging(const LogConfig&)
 * @brief initializes the SystemC logging system with a particular
 * configuration
 *
 * @param log_config the logging configuration
 */
void init_logging(const LogConfig& log_config);
/**
 * @fn void set_logging_level(log)
 * @brief sets the SystemC logging level
 *
 * @param level the logging level
 */
void set_logging_level(log level);
/**
 * @fn log get_logging_level()
 * @brief get the SystemC logging level
 *
 * @return the logging level
 */
log get_logging_level();
/**
 * @fn void set_cycle_base(sc_core::sc_time)
 * @brief sets the cycle base for cycle based logging
 *
 * if this is set to a non-SC_ZERO_TIME value all logging timestamps are
 * printed as cyles (multiple of this value)
 *
 * @param period the cycle period
 */
void set_cycle_base(sc_core::sc_time period);
/**
 * @fn sc_core::sc_verbosity get_log_verbosity()
 * @brief get the global verbosity level
 *
 * @return the global verbosity level
 */
inline sc_core::sc_verbosity get_log_verbosity() {
    return static_cast<sc_core::sc_verbosity>(
        ::sc_core::sc_report_handler::get_verbosity_level());
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
inline sc_core::sc_verbosity get_log_verbosity(std::string const& t) {
    return get_log_verbosity(t.c_str());
}

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
template <sc_core::sc_severity SEVERITY>
struct ScLogger {
    /**
     * @fn  ScLogger(const char*, int, int=sc_core::SC_MEDIUM)
     * @brief
     *
     * @param file where the log entry originates
     * @param line number where the log entry originates
     * @param verbosity the log level
     */
    ScLogger(const char* file, int line, int verbosity = sc_core::SC_MEDIUM):
        t(nullptr), file(file), line(line), level(verbosity){};

    ScLogger() = delete;

    ScLogger(const ScLogger&) = delete;

    ScLogger(ScLogger&&) = delete;

    ScLogger& operator=(const ScLogger&) = delete;

    ScLogger& operator=(ScLogger&&) = delete;
    /**
     * @fn  ~ScLogger()
     * @brief the destructor generating the SystemC report
     *
     */
    virtual ~ScLogger() {
        ::sc_core::sc_report_handler::report(
            SEVERITY, t ? t : "SystemC", os.str().c_str(), level, file, line);
    }
    /**
     * @fn ScLogger& type()
     * @brief reset the category of the log entry
     *
     * @return reference to self for chaining
     */
    inline ScLogger& type() {
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
    inline ScLogger& type(char const* t) {
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
    inline ScLogger& type(std::string const& t) {
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
#define SCP_LOGGER_NAME(x) CAT(_m_scp_log_level_cache_, x)

/* User interface macros */
#define SCMOD this->sc_core::sc_module::name()
#define SCP_LOGGER(...)                                                 \
    scp::scp_logger_cache IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))(        \
        SCP_LOGGER_NAME(EXPAND(FIRST_ARG FIRST_ARG(__VA_ARGS__))),      \
        SCP_LOGGER_NAME()) = { sc_core::SC_UNSET,                       \
                               "",                                      \
                               { IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))( \
                                   POP_ARG(__VA_ARGS__), ##__VA_ARGS__) } }

#define SCP_LOGGER_VECTOR(NAME) \
    std::vector<scp::scp_logger_cache> SCP_LOGGER_NAME(NAME)
#define SCP_LOGGER_VECTOR_PUSH_BACK(NAME, ...) \
    SCP_LOGGER_NAME(NAME).push_back(           \
        { sc_core::SC_UNSET, "", { __VA_ARGS__ } });

class call_sc_name_fn
{
    template <class T>
    static auto test(T* p)
        -> decltype(p->sc_core::sc_module::name(), std::true_type());
    template <class T>
    static auto test(...) -> decltype(std::false_type());

    template <class T>
    static constexpr bool has_method = decltype(test<T>(nullptr))::value;

public:
    // define a function IF the method exists
    template <class TYPE>
    auto operator()(TYPE* p) const
        -> std::enable_if_t<has_method<TYPE>, const char*> {
        return p->sc_core::sc_module::name();
    }

    // define a function IF NOT the method exists
    template <class TYPE>
    auto operator()(TYPE* p) const
        -> std::enable_if_t<!has_method<TYPE>, const char*> {
        return nullptr;
    }
};

// critical thing is that the initial if 'fails' as soon as possible - if it is
// going to pass, we have all the time we want, as we will be logging anyway
// This HAS to be done as a macro, because the first argument may be a string
// or a cache'd level

/*** Helper macros for SCP_ report macros ****/
#define SCP_VBSTY_CHECK_CACHED(lvl, features, cached, ...)             \
    (cached.level >= lvl) &&                                           \
        (cached.get_log_verbosity_cached(scp::call_sc_name_fn()(this), \
                                         typeid(*this).name()) >= lvl)

#define SCP_VBSTY_CHECK_UNCACHED(lvl, ...) \
    (::scp::get_log_verbosity(__VA_ARGS__) >= lvl)

#define SCP_VBSTY_CHECK(lvl, ...)                                    \
    IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))                            \
    (SCP_VBSTY_CHECK_CACHED(                                         \
         lvl, FIRST_ARG(__VA_ARGS__),                                \
         SCP_LOGGER_NAME(EXPAND(FIRST_ARG FIRST_ARG(__VA_ARGS__)))), \
     SCP_VBSTY_CHECK_UNCACHED(lvl, ##__VA_ARGS__))

#define SCP_GET_FEATURES(...)                                               \
    IIF(IS_PAREN(FIRST_ARG(__VA_ARGS__)))                                   \
    (FIRST_ARG EXPAND((POP_ARG(                                             \
         __VA_ARGS__,                                                       \
         SCP_LOGGER_NAME(EXPAND(FIRST_ARG FIRST_ARG(__VA_ARGS__))).type))), \
     __VA_ARGS__)

#ifdef FMT_SHARED
#define _SCP_FMT_EMPTY_STR(...) fmt::format(__VA_ARGS__)
#else
#define _SCP_FMT_EMPTY_STR(...) "Please add FMT library for FMT support."
#endif

#define SCP_LOG(lvl, ...)                                             \
    ::scp::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, lvl / 10) \
            .type(SCP_GET_FEATURES(__VA_ARGS__))                      \
            .get()                                                    \
        << _SCP_FMT_EMPTY_STR
/*** End HELPER Macros *******/

//! macro for debug trace level output
#define SCP_TRACEALL(...)                                  \
    if (SCP_VBSTY_CHECK(sc_core::SC_DEBUG, ##__VA_ARGS__)) \
    SCP_LOG(sc_core::SC_DEBUG, __VA_ARGS__)
//! macro for trace level output
#define SCP_TRACE(...)                                    \
    if (SCP_VBSTY_CHECK(sc_core::SC_FULL, ##__VA_ARGS__)) \
    SCP_LOG(sc_core::SC_FULL, __VA_ARGS__)
//! macro for debug level output
#define SCP_DEBUG(...)                                    \
    if (SCP_VBSTY_CHECK(sc_core::SC_HIGH, ##__VA_ARGS__)) \
    SCP_LOG(sc_core::SC_HIGH, __VA_ARGS__)
//! macro for info level output
#define SCP_INFO(...)                                       \
    if (SCP_VBSTY_CHECK(sc_core::SC_MEDIUM, ##__VA_ARGS__)) \
    SCP_LOG(sc_core::SC_MEDIUM, __VA_ARGS__)
//! macro for warning level output
#define SCP_WARN(...)                                          \
    if (SCP_VBSTY_CHECK(sc_core::SC_LOW, ##__VA_ARGS__))       \
    ::scp::ScLogger<::sc_core::SC_WARNING>(__FILE__, __LINE__, \
                                           sc_core::SC_MEDIUM) \
            .type(SCP_GET_FEATURES(__VA_ARGS__))               \
            .get()                                             \
        << _SCP_FMT_EMPTY_STR
//! macro for error level output
#define SCP_ERR(...)                                         \
    ::scp::ScLogger<::sc_core::SC_ERROR>(__FILE__, __LINE__, \
                                         sc_core::SC_MEDIUM) \
            .type(SCP_GET_FEATURES(__VA_ARGS__))             \
            .get()                                           \
        << _SCP_FMT_EMPTY_STR
//! macro for fatal message output
#define SCP_FATAL(...)                                       \
    ::scp::ScLogger<::sc_core::SC_FATAL>(__FILE__, __LINE__, \
                                         sc_core::SC_MEDIUM) \
            .type(SCP_GET_FEATURES(__VA_ARGS__))             \
            .get()                                           \
        << _SCP_FMT_EMPTY_STR

#ifdef NDEBUG
#define SCP_ASSERT(expr) ((void)0)
#else
#define SCP_ASSERT(expr)                                                  \
    ((void)((expr) ? 0                                                    \
                   : (SC_REPORT_FATAL(::sc_core::SC_ID_ASSERTION_FAILED_, \
                                      #expr),                             \
                      0)))
#endif

} // namespace scp
/** @} */ // end of scp-report
#endif    /* _SCP_REPORT_H_ */
