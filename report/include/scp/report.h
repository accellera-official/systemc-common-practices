/* this is merely a convenience to maintain the old API's */

#include "scp/sc_report.h"
#include "scp/helpers.h"
#include "scp/logger.h"

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
#ifndef _GLOBAL_SCP_LOGGER_FROM_CCI_DEFINED
#define _GLOBAL_SCP_LOGGER_FROM_CCI_DEFINED
#include "scp/report_cci_setter.h"
static scp::scp_logger_from_cci _global_scp_logger_from_cci;
#endif // _GLOBAL_SCP_LOGGER_FROM_CCI_DEFINED
#endif
