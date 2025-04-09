
/* this is merely a convenience to maintain the old API's used by SCP. It should be removed once the logger library is
 * upstreamed */

#ifndef _SCP_BW_REPORT_H_
#define _SCP_BW_REPORT_H_

#include "sc_log/sc_log.h"
#include "scp/helpers.h"
#include "scp/logger.h"

/* Only for backwards compatibility with SCP library */
#ifdef SC_HAS_SC_LOG
#define _SCP_FMT_EMPTY_STR _SC_LOG_FMT_EMPTY_STR
#endif

#define SCP_LOG_LEVEL_PARAM_NAME SC_LOG_LEVEL_PARAM_NAME

#define SCP_FATAL(...)                                                                          \
    ::sc_log::ScLogger<::sc_core::SC_FATAL, true>(__FILE__, __LINE__, sc_log::log_levels::NONE) \
            .type(SCP_GET_FEATURES(__VA_ARGS__))                                                \
            .get()                                                                              \
        << _SCP_FMT_EMPTY_STR
#define SCP_ERR(...)                                                                            \
    ::sc_log::ScLogger<::sc_core::SC_ERROR, true>(__FILE__, __LINE__, sc_log::log_levels::NONE) \
            .type(SCP_GET_FEATURES(__VA_ARGS__))                                                \
            .get()                                                                              \
        << _SCP_FMT_EMPTY_STR
#define SCP_WARN(...)     SC_LOG_AT(sc_log::log_levels::WARN, __VA_ARGS__)
#define SCP_INFO(...)     SC_LOG_AT(sc_log::log_levels::INFO, __VA_ARGS__)
#define SCP_DEBUG(...)    SC_LOG_AT(sc_log::log_levels::DEBUG, __VA_ARGS__)
#define SCP_TRACE(...)    SC_LOG_AT(sc_log::log_levels::TRACE, __VA_ARGS__)
#define SCP_TRACEALL(...) SC_LOG_AT(sc_log::log_levels::TRACE, __VA_ARGS__)
#define SCP_FULL(...)     SC_LOG_AT(sc_log::log_levels::TRACE, __VA_ARGS__)

#define SCP_LOGGER(...)                        SC_LOG_HANDLE(__VA_ARGS__)
#define SCP_LOGGER_NAME(...)                   SC_LOG_HANDLE_NAME(__VA_ARGS__)
#define SCP_LOGGER_VECTOR(NAME)                SC_LOG_HANDLE_VECTOR(NAME)
#define SCP_LOGGER_VECTOR_PUSH_BACK(NAME, ...) SC_LOG_HANDLE_VECTOR_PUSH_BACK(NAME, __VA_ARGS__)

namespace scp {
/* provide weekly typed enum constants for backwards compatibility */
enum log {
    NONE = sc_core::SC_NONE,
    CRITICAL = sc_core::SC_NONE,
    WARN = sc_core::SC_LOW,
    INFO = sc_core::SC_MEDIUM,
    DEBUG = sc_core::SC_HIGH,
    TRACE = sc_core::SC_DEBUG,
    DBGTRACE = sc_core::SC_DEBUG,
    FULL = sc_core::SC_DEBUG,
};
using scp_logger_cache = sc_log::sc_log_logger_cache;
using LogConfig = sc_log::LogConfig;
inline namespace sc_log {
using namespace ::sc_log;
}
} // namespace scp

#ifdef FMT_SHARED
#ifndef _FMT_CONVENIENCE_DEFINED
#define _FMT_CONVENIENCE_DEFINED
#include <fmt/format.h>

/* Convenience for common SystemC useage */
// Allows Join to be used on vectors etc.
#include <fmt/ranges.h>
// cover 'std' types
#include <fmt/std.h>
// Make all enum's behave like their underlying types by default
#include <type_traits>
template <typename E>
struct fmt::formatter<E, std::enable_if_t<std::is_enum_v<std::decay_t<E>>, char>> : fmt::formatter<std::string_view> {
    auto format(E e, format_context& ctx) const { return formatter<std::string_view, char>::format("Test", ctx); }
};
#endif // _FMT_CONVENIENCE_DEFINED
#endif

#ifdef HAS_CCI
#ifndef _GLOBAL_SC_LOG_LOGGER_FROM_CCI_DEFINED
#define _GLOBAL_SC_LOG_LOGGER_FROM_CCI_DEFINED
#include "scp/report_cci_setter.h"
static scp::scp_logger_from_cci _global_sc_log_logger_from_cci;
#endif // _GLOBAL_SCP_LOGGER_FROM_CCI_DEFINED
#endif

#endif // _SCP_BW_REPORT_H_