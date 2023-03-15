

## Reporting macros

The library offers the following macros. These macros ensure that, in general, a single 'if' is used to guard against output, so they can be used liberally throughout model code.

The library uses the Cmake Package Manager to fetch SystemC, CCI, RapidJSON, FMT Library, Spdlog.
For SystemC and CCI by default we use the "Master" Branch. Use a package lock to set a specific version if you need to.

see: https://github.com/cpm-cmake/CPM.cmake

----

The Log levels used by the scp library are as follows :

| SCP log <br /> level value | SCP_ report macro | Log levels name | Print Level | Equivelent sc_core |
| --- | --- | --- | --- | --- |
| 0 |   NONE    |   | sc_core::SC_NONE |
| 1 | SCP_FATAL()    | FATAL    | sc_core::SC_LOW   | `sc_core::SC_FATAL` (Always printed)
| 1 | SCP_ERROR()    | ERROR    | sc_core::SC_LOW   | `sc_core::SC_ERROR` (Always printed)
| 1 | SCP_WARNING()  | WARNING  | sc_core::SC_LOW   | `sc_core::SC_WARNING`
| 4 | SCP_INFO()     | INFO     | sc_core::SC_MEDIUM| `sc_core::SC_MEDIUM`
| 5 | SCP_DEBUG()    | DEBUG    | sc_core::SC_HIGH  | `sc_core::SC_HIGH`
| 6 | SCP_TRACE()    | TRACE    | sc_core::SC_FULL  | `sc_core::SC_FULL`
| 7 | SCP_TRACEALL() | TRACEAL  | sc_core::SC_DEBUG | `sc_core::SC_DEBUG`

Hence WARNINGS will be printed if the log level is set above 1. Hence setting a log_level of 3 will print Fatal, Error and Warning messages only.

## SCP_ report macros

The following SCP_ report macros can process an [{FMT}](https://github.com/fmtlib/fmt) formatter, or operate as a normal stream (accepting normal operators for output).
```C
    SCP_TRACE() << "My trace message";
    SCP_TRACE()("The answer is {}.", 42);
```

The macros can take the following options:

```C
    SCP_TRACE()
```
Uses the global default report level set up during initialization to determine whether the message is printed. (Defaults to 'SC_WARNING' in the absence of any initialization). No 'feature' information will be printed with the message.


```C
    SCP_TRACE("string")
```
The string represents a `feature` that is being reported upon. The message will be tagged with the feature name, and the feature name will be printed along with the message. The name will also be used to look up a `CCI` parameter with the extension `log_level`.
Hence:
```C
   SCP_TRACE("top.mymodel")
```
will check for, in order of priority:
    `top.mymodel.log_level`
    `top.log_level`
    `log_level`
The first matching parameter will be used. Hence it is possible to set a top level `top.log_level` and overwrite that for specific models in the hierarchy (`top.mymodel.log_level`).
If no parameters with matching names are found, the global default report level set up during initialization will be used (Defaults to 'SC_WARNING' in the absence of any initialization).

Any string can be used, and it maybe a `std::string` or a `const char*`. Hence, users may create hierarchies of reporting features outside of the SystemC hierarchy itself. None the less, a convenience macro `SCMOD` is provided to use the current `SC_MODULE` name. Hence a typical example would be
```C
    SCP_TRACE(SCMOD) << "My trace message";
```
Having established whether the feature should be printed or not, the result is cached in a lookup table. This lookup table will be used on all subsequent calls to any macro using the same feature string. (see thread safety below)

This form of `SCP_TRACE` uses a global lookup table. This means there is a look-up 'cost' each time an SCP_ report function is used. Also see below for thread safety concerns. (Only one string is permitted in this form, because it will be 'hashed' and used to look up in the table)

```C
   SCP_TRACE((logger))  
```
In this form, the `logger` is expected to be within scope of the macro, it should be instantiated using the `SCP_LOGGER` macro. It will be used to store the debug level at which printing should occur and feature information which will be shared by all users of the `logger`. An alternate form makes use of the default logger (see below) : `SCP_TRACE(())`.

```C
    SCP_TRACE((logger),"string")
```
Both forms can be used together, in which case the logging type used in the output will be the feature string, while the logger will be used to determine if the 

```C
   SCP_LOGGER()
```
The `SCP_LOGGER` macro is used to instantiate a logger for use in an SCP_ report macro. It is expected to be used in a class definition and the logger is expected to be used within that class. It should be constructed with the 'features' of logging for which it will be used. There are several forms of the SCP_LOGGER macro. With no arguments, the macro will construct a logger with the default name in the current scope. It is an error to use the macro more than once with no arguments.

By default, the logger will be initiated with some default features on the first use of any SCP_ report macro. This MUST happen within the SystemC context (on the SystemC thread) - it is safest to use an `SC_TRACE` macro (for instance) in the sc_module constructor.

The default features are the SystemC hierarchial name (`this->name()`) and the C++ type name.  The C++ type name is demangled, and will be pre-pended with the SystemC hierarchical name. 

Hence an sc_module `"my_mod"` instanced with the hierarchical name `top.a.b.mod` will automatically include the feature `top.a.b.mod.my_mod` and may be enabled using the parameter `top.a.b.mod.m_mod.log_level`. (See below for wildcard options).


```C
   SCP_LOGGER("string"...)
```
In this variant, the variable umber of feature strings passes will be *prepended* to the list of default features such that the features listed take precedence over the default features. The strings may be either `std::string`'s or `const char*`'s.

The feature names will be pre-pended with the SystemC hierarchical name (at the point the first SCP_ report macro is used).

Hence a string `"spacial"` used in model `top.a.b` will be inserted interpreted as feature `top.a.b.special` and may be enabled using the parameter `top.a.b.special.log_level`. (See below for wildcard options).


```C
   SCP_LOGGER((my_logger))
```
In this form, `my_logger` will be used as the logger name, which should also be used as the logger name for the SCP_ report macros. The actual name of the instantiated variable will be 'mangled' such that there is no danger of name collision, hence short logger names are perfectly permissable, they may even be single digits (e.g. `SCP_LOGGER((1))` which would allow the use of report functions such as `SCP_TRACE((1))`. Omitting the logger name will be equivalent of the previous macros (hence `SCP_LOGGER(())` is the same as `SCP_LOGGER()`). These variants may be combined e.g. `SCP_LOGGER((1),"feature", "feature.sub_feature")`

## feature matching rules

A logger can be initialized with a variable number of feature strings, each of which may be used to identify features that can then be enabled using CCI parameters. The hierarchical SystemC decomposition is used to find the best match for a feature. The feature (and corresponding log level) that best matches (i.e is the closest in the hierarchy to the feature) will be used. In addition, CCI parameters who's name starts with `*.` can be used to match several levels of hierarchy.

Hence a module of type `mymod`, instanced with hierarchical name `top.foo` with feature `a.b` will search in order for:

|  Priority   |     |
| --- | --- |
|  4  |`top.foo.a.b.log_level`    |
|  3  |`top.foo.a.log_level`      |
|  2  |`top.foo.log_level`        |
|  4  |`*.foo.a.b.log_level`      |
|  3  |`*.foo.a.log_level`        |
|  2  |`*.foo.log_level`          |
|  3  |`top.a.b.log_level`        |
|  2  |`top.a.log_level`          |
|  1  |`top.log_level`            |
|  2  |`a.b.log_level`            |
|  2  |`*.b.log_level`            |
|  1  |`mymod.log_level`          |
|  1  |`*.log_level`              |
|  0  |`log_level`                |




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

| Use                                                        |   method                                   |  Default
| ---- | ---- | --- |
| set the logging level                                      |  `logLevel(int)`                | WARNING (3) |
| define the width of the message field, 0 to disable,  <br />`std::numeric_limits<unsigned>::max()` for arbitrary width     |  ` msgTypeFieldWidth(unsigned)`  | | 
| enable/disable printing of system time |    `printSysTime(bool)`       | true |
| enable/disable printing of simulation time |   `printSimTime(bool)`      | true |
| enable/disable printing delta cycles       |  `printDelta(bool)`          | true |
| enable/disable printing of severity level  |  `printSeverity(bool)`       | true |
| enable/disable colored output              |  `coloredOutput(bool)`        | true |
| set the file name for the log output file  |  `logFileName([const] std::string&)`  |  |
| set the regular expression to filter the output  |  `logFilterRegex([const] std::string&)` |  |
| enable/disable asynchronous output (write to file in separate thread  |  `logAsync(bool)` | true |
| print the file name from this log level |  `fileInfoFrom(int)` | sc_core::SC_INFO (4) |
| disable/enable the suppression of all error messages after the first  |    `reportOnlyFirstError(bool)` | true |

## Thread safety

None of the macro's are thread safe. SCP_LOGGER *must* be used within a SystemC module context. The SCP_ report macros MAY be used outside of a SystemC module context, and may be used on separate threads. However they *must* first be used on the SystemC thread within a module context. 

Hence it is recommended that every sc_module constructor includes something like:
```C
    SCP_TRACE(()) << "Constructor";
```
(This will print the module hierarchy name as well as other information so the short message string is still useful.)

This is equally true whether using a local 'logger' or the global lookup table. When using the global lookup table, in separate threads, care has to be taken that NO logger is added once reporting starts on the non SystemC thread as this could potentially corrupt the lookup table (which is only thread safe for multiple reads). In general, it is highly recommended to use the `(logger)` form for such cases.

## Recommendations

The recommended way to use this library is:

For Systems that are not using CCI parameters, use the SCP_ report macros with no parameters, either with or without a top level initialization. e.g. `SCP_TRACE() << "your message";`

For modules that need very occasional reporting can use the "string" form of the SCP_ report macros, ideally using SCMOD. e.g. `SCP_TRACE(SCMOD) << "your message";`

For modules that either use a lot of reporting, or require multiple threads, use the `(logger)` form, e.g. `SCP_TRACE(()) << "your message";`. This will require instantiating the logger which can be done with `SCP_LOGGER(())` in the class. 

For Systems and modules that have specific features which do not align with the module hierarchy, the `(logger)` form should be used, with additional feature strings.

For the user, reporting can be enabled using CCI parameters. This can be achieved in most cases on the command line. To enable or disable specific modules in the hierarchy the full hierarchy name can be used e.g. `-p top.a.b.my_module.log_level=5`. To enable all instances of a specific model the wildcard can be used e.g. `-p *.model.log_level=5`. A global 'default' can be provided at the top level.


## Advanced Arrays

Sometimes it's important to use both a 'cached' approach, and to allow the cache to be (locally) dynamic. For instance as now client devices are added to a tlm multi-port, it may be important to log messages per client. To achieve this 2 macros are provided:

```C
    SCP_LOGGER_VECTOR(NAME)
```
This instantiates a vector of loggers (with the base name `NAME`).

```C
    SCP_LOGGER_VECTOR_PUSH_BACK(NAME, "features"... )
```
This will push_back a new logger to the logger vector, initialized with the features listed.
From this point on any of the SCP_ reporting macro's can be used with the form `SCP_INFO((NAME[i]))`

## Utilities

A utility function is provided to list all the logging parameters available in the system. This can be used for help messages for instance.
```C
    std::vector<std::string> get_logging_parameters();
```
