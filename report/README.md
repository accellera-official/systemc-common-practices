
***
NB the TROW actions in sc_report handler should be marked as deprecated, and users encouraged to calling throw as appropriate in their user code. This removes the ambiguity about whether and what a report handler would throw.
***

#Problem Statement and proposed Solution

This part of the library is intended to provide a convenient way to log information, such that it is easy to adopt and, in doing so, eases the path to code-reuse, (typically "logging libraries" are one of the key common parts which need to be included from each different code base).

The goals are:
1/ A simple interface for model writers that supports both "{std::format}" style syntax, available in C++20, such as `"Hello {}","world"` and also streaming syntax `<<"Hello "<<"world"`. 
2/ Provided a mechanism to allow run-time enabling of logging, which can be (for instance) connected to the SystemC CCI standard for configuration.
3/ The interface should be efficient, to encourage model writers to be able to use as much logging as they require, without the need to remove it for production code. Hence a single `if` should be used to checked against the logging level prior to evaluating the full contents of the logging message, such that computationally expensive calls can be used to build the message, and will have no effect on simulation speed if they are not reported.

# Implementation details
The implementation of the Logging interface provides a short lived streaming object which collects the information required for logging, and will then pass that to a sc_report_handler. This object should not be used directly, rather the macros described below should be preferred.

***The logging interface can _NOT_ be used to cause exceptions to be thrown, this should be handled in user code (The underlying sc_report_handler sc_core::SC_THROW, sc_core::SC_INTERRUPT, sc_core::SC_STOP and sc_core::SC_ABORT actions will be masked). ***

## Loggin interface

the following levels of logging are pre-defined:
    `CRITICAL` : Equivalent to SystemC verbosity `sc_core::SC_NONE`
    `WARN` : Equivalent to SystemC verbosity `sc_core::SC_LOW`
    `INFO` : Equivalent to SystemC verbosity `sc_core::SC_MEDIUM`
    `DEBUG` : Equivalent to SystemC verbosity `sc_core::SC_HIGH`
    `TRACE` : Equivalent to SystemC verbosity `sc_core::SC_DEBUG`

The following macros are provided:

`SCMOD` : Convenience for `this->sc_core::sc_module::name()`
`SC_LOG_HANDLE(...)` : Define a log handle, that can be used in subsequent LOG macros.
`SC_LOG_HANDLE_VECTOR(NAME)` : Define a vector of handles.
`SC_LOG_HANDLE_VECTOR_PUSH_BACK(NAME, ...)` : Add to a vector of handles.

`SC_LOG_AT(lvl, ...)` : Log at level `lvl`
`SC_CRITICAL(...)` : Log at level `CRITICAL`
`SC_WARN(...)` : Log at level `WARN`
`SC_INFO(...)` : Log at level `INFO`
`SC_DEBUG(...)` : Log at level `DEBUG`
`SC_TRACE(...)` : Log at level `TRACE`


## sc_log macros

The following SC_ report macros can process an `{std::format}` formatter, or operate on a stream
```C
    SC_TRACE() << "My trace message";
    SC_TRACE()("The answer is {}.", 42);
```

The macros can take the following options (`SC_TRACE` is used as an example, but the forms apply to all macros `SC_LOG_AT(lvl, ...)`, `SC_CRITICAL(...)`, `SC_WARN(...)`, `SC_INFO(...)`, `SC_DEBUG(...)` and `SC_TRACE(...)`):

```C
    SC_TRACE()
```
Uses the global default report level as returned by ::sc_core::sc_report_handler::get_verbosity_level() to determine whether the message is printed. (This typically defaults to `SC_LOW` in the absence of any initialization). No 'feature' information will be included with the message.


```C
    SC_TRACE("string")
```
The string represents a `feature` that is being reported upon. The message will be tagged with the feature name, and the feature name will be included along with the message. The name will also be used to look up the log level, in order to determine whether the message should be reported or not.

Any string can be used, and it maybe a `std::string` or a `const char*`. Hence, users may create hierarchies of reporting features outside of the SystemC hierarchy itself. The convenience macro `SCMOD` is provided to use the current `SC_MODULE` name. Hence a typical example could be
```C
    SC_TRACE(SCMOD) << "My trace message";
```
Having established whether the feature should be printed or not, the result is cached in a lookup table. This lookup table will be used on all subsequent calls to any macro using the same feature string. (see thread safety below)

**NOTE** Nonetheless, there is a cost of this (string based) lookup. Hence other forms are preferred for critical code.

```C
   SC_TRACE((handle))  
```
In this form, the `handle` is expected to be within scope when the macro is used, it should be instantiated using the `SC_LOG_HANDLE` macro (see below). It will be used to store the debug level at which printing should occur and feature information which will be shared by all users of the `handle`.

**NOTE** For both thread safety and critical code regions, this form is preferred.

For convenience, a "default" handle is provided, and the alternate form makes use of the default handle (see below) : `SC_TRACE(())`.

```C
    SC_TRACE((handle),"string")
```
Both forms can be used together, in which case the logging type used in the output will be the feature string, while the handle will be used to determine if the message should be reported.

```C
   SC_LOG_HANDLE()
```
The `SC_LOG_HANDLE` macro is used to instantiate a handle for use in the logging macros. It is expected to be used in a class definition and the handle is expected to be used within that class. It should be constructed with the "features" of logging for which it will be used. There are several forms of the SC_LOG_HANDLE macro. With no arguments, the macro will construct a handle with the "default" name in the current scope. It is an error to use the macro more than once with no arguments.

By default, the handle will be initiated on the first use of any SC_ report macro, and the log level at which messages are reported will be evaluated at this point (and remain constant from then on). ThFor thread safety, this *MUST* happen within the SystemC context (on the SystemC thread) - it is safest to use an `SC_TRACE` macro (for instance) in the sc_module constructor.

The default features are the SystemC hierarchial name (`this->name()`) and the C++ type name.  The C++ type name is demangled, and will be pre-pended with the SystemC hierarchical name. 

Hence an sc_module `"my_mod"` instanced with the hierarchical name `top.a.b.mod` will automatically include the feature `top.a.b.mod.my_mod`. The CCI interface mechanism (see below) can be used to enable this feature, by using the parameter `top.a.b.mod.m_mod.log_level`.


```C
   SC_LOG_HANDLE("string"...)
```
In this variant, the variable number of feature strings passes will be *prepended* to the list of default features such that the features listed take precedence over the default features. The strings may be either `std::string`'s or `const char*`'s.

The feature names will be pre-pended with the SystemC hierarchical name (at the point the first SC_ report macro is used).

Hence a string `"spacial"` used in model `top.a.b` will be inserted as feature `top.a.b.special` (and using CCI may be enabled using the parameter `top.a.b.special.log_level`).


```C
   SC_LOG_HANDLE((my_handle))
```
In this form, `my_handle` will be used as the handle name, which should also be used as the handle name for the SC_ report macros. The actual name of the instantiated variable will be 'mangled' such that there is no danger of name collision, hence short handle names are perfectly permissible, they may even be single digits (e.g. `SC_LOG_HANDLE((1))` which would allow the use of report functions such as `SC_TRACE((1))`). Omitting the handle name will be equivalent of the previous macros (hence `SC_LOG_HANDLE(())` is the same as `SC_LOG_HANDLE()`). These variants may be combined e.g. `SC_LOG_HANDLE((1),"feature", "feature.sub_feature")`

## Report setters and CCI Integration 

A special sc_object is defined called sc_log_global_logger_handler
```C
struct sc_log_global_logger_handler : sc_core::sc_object {
    virtual log_levels operator()(struct sc_log_logger_cache& logger, const char* scname,
                                             const char* tname) const = 0;
};
```
Calls to this function will return the log level for a specific log handle, a systemc hierarchical name, and a type name. Users may implement their own derived instances of this class to determine whether messages should be reported.

The SCP library contains an instance of this class which uses SyetemC::CCI parameters to determine this information. The following documentation is included here for convenience.

## CCI Integration (in thr SCP library)

The SCP implementation of the global logger handler makes use of CCI parameters suffixed with the extension `log_level`.
Hence:
```C
   SC_TRACE("top.mymodel")
```
will check for, in order of priority:
    `top.mymodel.log_level`
    `top.log_level`
    `log_level`
The first matching parameter will be used. Hence it is possible to set a top level `top.log_level` and overwrite that for specific models in the hierarchy (`top.mymodel.log_level`).
If no parameters with matching names are found, the global default report level set up during initialization will be used (Defaults to 'WARN' in the absence of any initialization).

A handle can be initialized with a variable number of feature strings, each of which may be used to identify features that can then be enabled using CCI parameters. The hierarchical SystemC decomposition is used to find the best match for a feature. The feature (and corresponding log level) that best matches (i.e is the closest in the hierarchy to the feature) will be used. In addition, CCI parameters who's name starts with `*.` can be used to match several levels of hierarchy.

Hence a module of type `mymod`, instanced with hierarchical name `top.foo` with feature `a.b` will search in order for:

| Priority |                         |
| -------- | ----------------------- |
| 4        | `top.foo.a.b.log_level` |
| 3        | `top.foo.a.log_level`   |
| 2        | `top.foo.log_level`     |
| 4        | `*.foo.a.b.log_level`   |
| 3        | `*.foo.a.log_level`     |
| 2        | `*.foo.log_level`       |
| 3        | `top.a.b.log_level`     |
| 2        | `top.a.log_level`       |
| 1        | `top.log_level`         |
| 2        | `a.b.log_level`         |
| 2        | `*.b.log_level`         |
| 1        | `mymod.log_level`       |
| 1        | `*.log_level`           |
| 0        | `log_level`             |




## Initialization
```C
   scp::init_logging(config)
```
This is an optional function which enables the more complex logging mechanisms. Without it, only basic logging is possible. This takes a configuration structure as a parameter.

The configuration structure can be constructed simply:
```C
    scp::LogConfig()
```
Convenience functions are provided on a configuration structure that return a modified structure, hence  for example:
```C
    scp::init_logging(
        scp::LogConfig()
            .logLevel(scp::log::DEBUG)
            .msgTypeFieldWidth(20)
            .fileInfoFrom(5)
            .logAsync(false)
            .printSimTime(false)
            .logFileName(logfile));
```

| Use                                                                                                                    | method                                 | Default              |
| ---------------------------------------------------------------------------------------------------------------------- | -------------------------------------- | -------------------- |
| set the logging level                                                                                                  | `logLevel(int)`                        | WARNING (3)          |
| define the width of the message field, 0 to disable,  <br />`std::numeric_limits<unsigned>::max()` for arbitrary width | ` msgTypeFieldWidth(unsigned)`         |                      |
| enable/disable printing of system time                                                                                 | `printSysTime(bool)`                   | true                 |
| enable/disable printing of simulation time                                                                             | `printSimTime(bool)`                   | true                 |
| enable/disable printing delta cycles                                                                                   | `printDelta(bool)`                     | true                 |
| enable/disable printing of severity level                                                                              | `printSeverity(bool)`                  | true                 |
| enable/disable colored output                                                                                          | `coloredOutput(bool)`                  | true                 |
| set the file name for the log output file                                                                              | `logFileName([const] std::string&)`    |                      |
| set the regular expression to filter the output                                                                        | `logFilterRegex([const] std::string&)` |                      |
| enable/disable asynchronous output (write to file in separate thread                                                   | `logAsync(bool)`                       | true                 |
| print the file name from this log level                                                                                | `fileInfoFrom(int)`                    | sc_core::SC_INFO (4) |
| disable/enable the suppression of all error messages after the first                                                   | `reportOnlyFirstError(bool)`           | true                 |

## Thread safety

None of the macro's are thread safe. SC_LOG_HANDLE *must* be used within a SystemC module context. The SC_ report macros MAY be used outside of a SystemC module context, and may be used on separate threads. However they *must* first be used on the SystemC thread within a module context. 

Hence it is recommended that every sc_module constructor includes something like:
```C
    SC_TRACE(()) << "Constructor";
```
(This will print the module hierarchy name as well as other information so the short message string is still useful.)

This is equally true whether using a local 'handle' or the global lookup table. When using the global lookup table, in separate threads, care has to be taken that NO handle is added once reporting starts on the non SystemC thread as this could potentially corrupt the lookup table (which is only thread safe for multiple reads). In general, it is highly recommended to use the `(handle)` form for such cases.

## Recommendations

The recommended way to use this library is:

For Systems that are not using CCI parameters, use the SC_ report macros with no parameters, either with or without a top level initialization. e.g. `SC_TRACE() << "your message";`

For modules that need very occasional reporting can use the "string" form of the SC_ report macros, ideally using SCMOD. e.g. `SC_TRACE(SCMOD) << "your message";`

For modules that either use a lot of reporting, or require multiple threads, use the `(handle)` form, e.g. `SC_TRACE(()) << "your message";`. This will require instantiating the handle which can be done with `SC_LOG_HANDLE(())` in the class. 

For Systems and modules that have specific features which do not align with the module hierarchy, the `(handle)` form should be used, with additional feature strings.

For the user, reporting can be enabled using CCI parameters. This can be achieved in most cases on the command line. To enable or disable specific modules in the hierarchy the full hierarchy name can be used e.g. `-p top.a.b.my_module.log_level=5`. To enable all instances of a specific model the wildcard can be used e.g. `-p *.model.log_level=5`. A global 'default' can be provided at the top level.


## Advanced Arrays

Sometimes it's important to use both a 'cached' approach, and to allow the cache to be (locally) dynamic. For instance as now client devices are added to a tlm multi-port, it may be important to log messages per client. To achieve this 2 macros are provided:

```C
    SC_LOG_HANDLE_VECTOR(NAME)
```
This instantiates a vector of handles (with the base name `NAME`).

```C
    SC_LOG_HANDLE_VECTOR_PUSH_BACK(NAME, "features"... )
```
This will push_back a new handle to the handle vector, initialized with the features listed.
From this point on any of the SC_ reporting macro's can be used with the form `SC_INFO((NAME[i]))`

## Utilities

A utility function is provided to list all the logging parameters available in the system. This can be used for help messages for instance.
```C
    std::vector<std::string> get_logging_parameters();
```
