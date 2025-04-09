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

#ifndef _SCP_LOGGER_H_
#define _SCP_LOGGER_H_

#include <systemc>
#include "sc_log/sc_log.h"

/** \ingroup scp-report
 *  @{
 */
/**@{*/
//! @brief reporting backend utilities
namespace sc_log {
/**
 * @fn void init_logging(log_levels=log_levels::WARN, unsigned=24, bool=false)
 * @brief initializes the SystemC logging system with a particular logging
 * level
 *
 * @param level the log level
 * @param type_field_width the with of the type field in the output
 * @param print_time whether to print the system time stamp
 */
void init_logging(log_levels level = log_levels::WARN, unsigned type_field_width = 24, bool print_time = false);
/**
 * @fn void init_logging(log=log::WARN, unsigned=24, bool=false)
 * @brief initializes the SystemC logging system with a particular logging
 * level
 *
 * @param level the log level
 * @param type_field_width the with of the type field in the output
 * @param print_time whether to print the system time stamp
 */
void reinit_logging(log_levels level = log_levels::WARN);
/**
 * @struct LogConfig
 * @brief the configuration class for the logging setup
 *
 * using this class allows to configure the logging output in many aspects. The
 * class follows the builder pattern.
 */
struct LogConfig {
    log_levels level{ log_levels::WARN };
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
    LogConfig& logLevel(log_levels);
    LogConfig& logLevel(int);
    //! define the width of the message field, 0 to disable,
    //! std::numeric_limits<unsigned>::max() for arbitrary width
    LogConfig& msgTypeFieldWidth(unsigned);
    //! enable/disable printing of system time
    LogConfig& printSysTime(bool = true);
    //! enable/disable printing of simulation time on console
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
void set_logging_level(log_levels level);
/**
 * @fn log get_logging_level()
 * @brief get the SystemC logging level
 *
 * @return the logging level
 */
log_levels get_logging_level();
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

} // namespace sc_log
/** @} */ // end of scp logger
#endif    /* _SCP_LOGGER_H_ */
