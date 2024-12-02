/* this is merely a convenience to maintain the old API's */

#include "scp/sc_report.h"
#include "scp/helpers.h"
#include "scp/logger.h"

#ifdef HAS_CCI
#ifndef _GLOBAL_SCP_LOGGER_FROM_CCI_DEFINED
#define _GLOBAL_SCP_LOGGER_FROM_CCI_DEFINED
#include "scp/report_cci_setter.h"
static scp::scp_logger_from_cci _global_scp_logger_from_cci;
#endif // _GLOBAL_SCP_LOGGER_FROM_CCI_DEFINED
#endif
