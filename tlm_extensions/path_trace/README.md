# Path Trace Extension

This extension provides a vector of SystemC objects. The expectation is that each SystemC model will 'stamp' the transaction as it passes through a network.
The extension provides the following API:

`void stamp(sc_core::sc_object* obj)`
: Stamp object obj into the Path Trace

`void reset()`
: Clear the Path trace, (eg. before returning it to a memeory allocation pool)
 
`std::string to_string(std::string separator = "->")`
:Convert the Path trace to a string, using the separator (default "->")


