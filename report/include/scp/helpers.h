/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

*****************************************************************************/

#include <systemc>
#include <tlm>
#ifndef _SCP_HELPERS_H_
#define _SCP_HELPERS_H_
namespace scp {
static std::string scp_txn_tostring(tlm::tlm_generic_payload& trans) {
    std::stringstream info;
    const char* cmd = "UNKOWN";
    switch (trans.get_command()) {
    case tlm::TLM_IGNORE_COMMAND:
        cmd = "IGNORE";
        break;
    case tlm::TLM_WRITE_COMMAND:
        cmd = "WRITE";
        break;
    case tlm::TLM_READ_COMMAND:
        cmd = "READ";
        break;
    }
    info << cmd << " to address: "
         << "0x" << std::hex << trans.get_address();
    info << " len: " << trans.get_data_length();
    unsigned char* ptr = trans.get_data_ptr();
    info << " data: 0x";
    for (int i = trans.get_data_length(); i; i--) {
        info << std::setw(2) << std::setfill('0') << std::hex
             << (unsigned int)(ptr[i - 1]);
    }
    info << " status: " << trans.get_response_string() << " ";
    for (unsigned int i = 0; i < tlm::max_num_extensions(); i++) {
        if (trans.get_extension(i)) {
            info << " extn:" << i;
        }
    }
    return info.str();
}

} // namespace scp
#endif /* _SCP_HELPERS_H_ */