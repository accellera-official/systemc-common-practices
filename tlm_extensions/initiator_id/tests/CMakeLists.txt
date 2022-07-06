macro(run_test test)
    add_executable(${test} ${test}.cc)
    target_link_libraries(${test} scp::tlm_extensions::initiator_id SystemC::systemc)
    add_test(NAME ${test} COMMAND ${test})
endmacro()

run_test(smoke)
