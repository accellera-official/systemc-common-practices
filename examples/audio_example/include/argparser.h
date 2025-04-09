
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__
#include <getopt.h>
#include <systemc>
#include <cci_configuration>
#include <scp/report.h>

std::pair<std::string, std::string> get_key_val_args(const std::string& arg)
{
    std::stringstream ss(arg);
    std::string key, value;

    if (!std::getline(ss, key, '=')) {
        SCP_FATAL("Parser") << "parameter name not found!" << std::endl;
    }
    if (!std::getline(ss, value)) {
        SCP_FATAL("Parser") << "parameter value not found!" << std::endl;
    }

    SCP_INFO("Parser") << "Setting param " << key << " to value " << value;

    return std::make_pair(key, value);
}
void parseCommandLineWithGetOpt(const int argc, const char* const* argv)
{
    SCP_INFO("Parser") << "Parse command line  (" << argc << " arguments)";

    // getopt permutes argv array, so copy it to argv_cp
    std::vector<std::unique_ptr<char[]>> argv_cp;
    char* argv_cp_raw[argc];
    for (int i = 0; i < argc; i++) {
        size_t len = strlen(argv[i]) + 1; // count \0
        argv_cp.emplace_back(std::make_unique<char[]>(len));
        strcpy(argv_cp[i].get(), argv[i]);
        argv_cp_raw[i] = argv_cp[i].get();
    }

    // configure getopt
    optind = 0; // reset of getopt
    opterr = 0; // avoid error message for not recognized option

    // Don't add 'i' here. It must be specified as a long option.
    const char* optstring = "p:";

    struct option long_options[] = { { "param", required_argument, 0, 'p' }, // --param foo.baa=10
                                     { 0, 0, 0, 0 } };

    while (1) {
        int c = getopt_long(argc, argv_cp_raw, optstring, long_options, 0);
        if (c == EOF) break;

        switch (c) {
        case 'p': // -p and --param
        {
            auto param = get_key_val_args(std::string(optarg));
            cci::cci_get_global_broker(cci::cci_originator("ArgumentParser"))
                .set_preset_cci_value(param.first, cci::cci_value(cci::cci_value::from_json(param.second)));
            break;
        }

        default:
            /* unrecognized option */
            break;
        }
    }
}

#endif