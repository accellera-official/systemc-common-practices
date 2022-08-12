
# Common Practices Working group additions


## Collection of TLM Extensions

The collection of interfaces here are independent of each other. They provide 'common' functionality in an architecturally independent way.

## CCI Parameters

This is a list of parameter names and their meanings


| Applies to | parameter name | type | Description |
|:----------:| -------------- | ---- | ----------- |
Initiator socket | "initiator_id"  | uint64_t | The ID that should be applied to a transaction leaving this initiator (typically stamped on a initiator_id extension) |
Target socket | "address"   | int64_t/uint64_t | Base address of a target socket. A "signed" value should be interpreted correctly (this is convenient to be able to specify remaining, large, address ranges)
Target socket | "size"      | uint64_t | Size of address range of a target socket. The address and size pair specify the part of the address space which should be 'routed' to the target socket.
Target socket | "relative_addresses" | bool | Specifies that relative addresses should be used (which should be the default). In other words, the address space, as seen by the target socket, will start at 0.

## Reporting infrastructure

The reporting infrastructure consist of 2 part: the frontend on to of sc_report and the backend, replacing SystemC standard handler. Both can be used independently.

### Reporting frontend

The frontend consist of a set of macros which provide a std::ostream to log a message. The execution of the logging code is dependend on the loglevel thus not impacting performance if the message is not logged.

The macros take an optional argument which becomes the message type. If not is provided, the default 'SystemC' is being used. The following table outlines how the scp::log level map to the SystemC logging parameter.

| SCP log level | SystemC severity | SystemC verbosity |
|---------------|------------------|-------------------|
| SCCFATAL      | SC_FATAL         | -- |
| SCCERR        | SC_ERROR         | -- |
| SCCWARN       | SC_WARNING       | -- |
| SCCINFO       | SC_INFO          | SC_MEDIUM |
| SCCDEBUG      | SC_INFO          | SC_HIGH |
| SCCTRACE      | SC_INFO          | SC_FULL |
| SCCTRACEALL   | SC_INFO |         SC_DEBUG |

### Reporting backend

The backend is initialized using the short form:

```
scp::init_logging(scp::log::INFO);
```

or the long form

``` 
scp::init_logging(scp::LogConfig()
		.logLevel(scp::log::DEBUG) // set log level to debug
		.msgTypeFieldWidth(10));   // make the msg type column a bit tighter
```

which allows more configurability. For detail please check the header file report.h
In both case an alternate report handler is installed which which uses a tabular format and spdlog for writing. By default spdlog logs asyncronously to keep the performance impact low.
