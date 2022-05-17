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
 ****************************************************************************/

#ifndef _SCP_MASTERID_EXTENSION_H
#define _SCP_MASTERID_EXTENSION_H

#include <systemc>
#include <tlm>

namespace scp {

/**
 * @class Master ID recording TLM extension
 *
 * @brief Master ID recording TLM extension
 *
 * @details Ignorable Extension type that can be used to add a Master ID (uint64_t).
 *          This is typically used for e.g. evaluating exclusive accesses.
 */

class MasterIDExtension : public tlm::tlm_extension<MasterIDExtension> {
    uint64_t m_id;
public:
    MasterIDExtension(uint64_t id) { m_id=id; }
    MasterIDExtension(const MasterIDExtension&) = default;

    virtual tlm_extension_base* clone() const override
    {
        return new MasterIDExtension(*this);
    }

    virtual void copy_from(const tlm_extension_base& ext) override
    {
        const MasterIDExtension& other = static_cast<const MasterIDExtension&>(ext);
        *this = other;
    }

    operator uint64_t () {return m_id;};

#define overload(_OP) MasterIDExtension& operator _OP(const uint64_t id){this->m_id _OP id; return *this; }
overload(+=);
overload(-=);
overload(*=);
overload(/=);
overload(%=);
overload(&=);
overload(|=);
overload(^=);
overload(<<=);
overload(>>=);

    MasterIDExtension& operator=(const uint64_t id) {m_id=id; return *this;}
};
}
#endif
