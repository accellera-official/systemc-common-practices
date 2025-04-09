/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MIDIPLAYER__
#define __MIDIPLAYER__

#include <cci_configuration>
#include <portaudio.h>
#include <scp/report.h>
#include <systemc>
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"

#include "Midi.h"

SC_MODULE (midiplayer) {
    cci::cci_param<std::string> midifile;
    SCP_LOGGER();
    tlm_utils::simple_initiator_socket<midiplayer> socket;

    SC_CTOR (midiplayer) :
    midifile("midifile", "bwv972.mid", "Midi file to play")
        {
            SCP_TRACE(())("Constructor");
            SC_THREAD(reader);
        }

    void reader()
    {
        Midi f{ midifile.get_value().c_str() };

        auto& header = f.getHeader();
        auto& tracks = f.getTracks();

        SCP_DEBUG(()) << "File read";
        SCP_DEBUG(()) << "File contents:";
        SCP_DEBUG(()) << "	Header:";
        SCP_DEBUG(()) << "		File format: " << (int)header.getFormat();
        SCP_DEBUG(()) << "		Number of tracks: " << header.getNTracks();
        SCP_DEBUG(()) << "		Time division: " << header.getDivision();

        for (const auto& track : tracks) {
            SCP_DEBUG(()) << "	Track events:";
            auto& events = track.getEvents();

            for (const auto& trackEvent : events) {
                auto* event = trackEvent.getEvent();
                uint8_t* data;
                if (event->getType() == MidiType::EventType::MidiEvent) {
                    auto* midiEvent = (MidiEvent*)event;

                    SCP_DEBUG(()) << "		Midi event:";
                    auto status = midiEvent->getStatus();

                    if (status == MidiType::MidiMessageStatus::NoteOn) {
                        if (trackEvent.getDeltaTime().getData() > 0) {
                            SCP_DEBUG(()) << "\t\t\tLength: " << trackEvent.getDeltaTime().getData();
                            wait(trackEvent.getDeltaTime().getData(), sc_core::SC_MS);
                        }

                        SCP_DEBUG(()) << "\t\t\t" << "Note " << (midiEvent->getVelocity() ? "on " : "off ")
                                      << (int)midiEvent->getNote() << " on channel " << (int)midiEvent->getChannel();
                        auto n = std::make_tuple<bool, int, int>(midiEvent->getVelocity(), (int)midiEvent->getNote(),
                                                                 (int)midiEvent->getChannel());
                        tlm::tlm_generic_payload txn;
                        txn.set_command(tlm::TLM_IGNORE_COMMAND);
                        txn.set_data_ptr(reinterpret_cast<unsigned char*>(&n));
                        txn.set_data_length(sizeof(n));
                        txn.set_streaming_width(sizeof(n));
                        txn.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
                        sc_core::sc_time delta;
                        socket->b_transport(txn, delta);
                    } else if (status == MidiType::MidiMessageStatus::NoteOff) {
                        if (trackEvent.getDeltaTime().getData() > 0) {
                            SCP_DEBUG(()) << "\t\t\tLength: " << trackEvent.getDeltaTime().getData();
                            wait(trackEvent.getDeltaTime().getData(), sc_core::SC_MS);
                        }
                        SCP_DEBUG(()) << "\t\t\t" << "Note off " << (int)midiEvent->getNote() << " on channel "
                                      << (int)midiEvent->getChannel();
                        auto n = std::make_tuple<bool, int, int>(false, (int)midiEvent->getNote(),
                                                                 (int)midiEvent->getChannel());
                        tlm::tlm_generic_payload txn;
                        txn.set_command(tlm::TLM_IGNORE_COMMAND);
                        txn.set_data_ptr(reinterpret_cast<unsigned char*>(&n));
                        txn.set_data_length(sizeof(n));
                        txn.set_streaming_width(sizeof(n));
                        txn.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
                        sc_core::sc_time delta;
                        socket->b_transport(txn, delta);
                    } else {
                        SCP_DEBUG(()) << "\t\t\tStatus: 0x" << hex << (int)midiEvent->getStatus();
                        SCP_DEBUG(()) << "\t\t\tData: 0x" << midiEvent->getData() << dec;
                    }
                } else if (event->getType() == MidiType::EventType::SysExEvent) {
                    SCP_DEBUG(()) << "\t\tSystem exclusive event:";
                    SCP_DEBUG(()) << "\t\t\tStatus: 0x" << hex << ((MetaEvent*)event)->getStatus();
                    SCP_DEBUG(()) << "\t\t\tData: 0x";

                    data = ((MetaEvent*)event)->getData();
                    for (int i{ 0 }; i < ((MetaEvent*)event)->getLength(); ++i) {
                        SCP_DEBUG(()) << (int)data[i];
                    }
                    SCP_DEBUG(()) << dec;
                } else { // event->getType() == MidiType::EventType::MetaEvent
                    SCP_DEBUG(()) << "\t\tMeta event:";
                    SCP_DEBUG(()) << "\t\t\tStatus: 0x" << hex << ((MetaEvent*)event)->getStatus();
                    SCP_DEBUG(()) << "\t\t\tData: 0x";

                    data = ((MetaEvent*)event)->getData();
                    for (int i{ 0 }; i < ((MetaEvent*)event)->getLength(); ++i) {
                        SCP_DEBUG(()) << (int)data[i];
                    }
                    if (!((MetaEvent*)event)->getLength()) {
                        SCP_DEBUG(()) << "0";
                    }
                    SCP_DEBUG(()) << dec;
                }
            }
        }
        sc_core::sc_stop();
    }
};

#endif