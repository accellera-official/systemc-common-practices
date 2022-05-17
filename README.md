
# Common Practices Working group TLM extensions and CCI Parameters


## Collection of TLM Extensions

The collection of interfaces here are independent of each other. They provide 'common' functionality in an architecturally independent way.

## CCI Parameters

This is a list of parameter names and their meanings


| Applies to | parameter name | type | Description |
|:----------:| -------------- | ---- | ----------- |
Initiator socket | "MasterID"  | uint64_t | The ID that should be applied to a transaction leaving this master (typically stamped on a MasterID extension) |
Target socket | "Address"   | int64_t/uint64_t | Base address of a target socket. A "signed" value should be interpreted correctly (this is convenient to be able to specify remaining, large, address ranges)
Target socket | "Size"      | uint64_t | Size of address range of a target socket. The address and size pair specify the part of the address space which should be 'routed' to the target socket.
Target socket | "relative_addresses" | bool | Specifies that relative addresses should be used (which should be the default). In other words, the address space, as seen by the target socket, will start at 0.