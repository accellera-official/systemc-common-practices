

## Threading Utilities

The library offers the following utilities

# scp::scp_async_event
***NB this could be considered for standardization within the SystemC kernel as sc_async_event***

An ***scp_async_event*** is identical to an sc_event except that it can be notified in a thread safe manner from a different thread. In the same way that an ***sc_event*** may be notified many times, an ***scp_async_event*** makes no guarantee about the number of times the actual event listener is notified, but does guarantee that it will at least be notified after the last time that any of the thread safe notify methods are called.

The ***scp_async_event*** also can be considered as a ***sc_prim_channel***. In addition to the standard ***sc_event*** interface, the ***scp_async_event*** also provides convenience functions to attach and detach the primary channel from the SystemC kernel as a potential source of external events.

*** NOTE ***
An ***scp_async_event***  *MUST* be constructed on the SystemC thread. Failure to do so is an error and will have unexpected consequences.

```cpp
class scp_async_event : public sc_core::sc_prim_channel, public sc_core::sc_event
{
public:
    scp_async_event(bool start_attached = true);
    void async_attach_suspending();
    void async_detach_suspending();
    void enable_attach_suspending(bool);
}
```
Hence ***async_attach_suspending*** shall attach the primary channel to the SystemC kernel as a potential source of external events, and will therefore prevent the SystemC kernel from exiting. See ***sc_prim_channel::async_attach_suspending***. Likewise ***async_detach_suspending*** will detach the primary channel, while ***enable_attach_suspending*** shall attach if the boolean argument is true, else it will detach the channel.

On construction by default the channel will be ***attached*** to the kernel, but this may be prevented by passing ***false*** to the constructor.

Notifications of the event from within the same thread as the SystemC kernel shall be treated identically to notifications posted to a normal ***sc_event***. When notified from a different thread the delay posted in the notification of the ***scp_async_event*** shall be respected, but there is no guarantee that the SystemC time will not have advanced by the time the event is posted within the SystemC kernel, so it is recommended that this delay not be used. From a separate thread there is no means to perform an ***immediate*** notification, this will be treated as a delta notification (with SC_ZERO_TIME).

Calling ***triggered()*** from a outside the SystemC kernel thread is an error and an SC_ERROR_REPORT will be generated.

# scp::run_on_systemc
***NB The interface of this class could be considered for standardization within the SystemC kernel***

***run_on_systemc*** provides a convenience class that can be instanced in order to provide the means to execute a ```std::function``` on the SystemC thread in a thread safe manner from another thread.

*** NOTE ***
An instance of the ***run_on_systemc*** module *MUST* be constructed on the SystemC thread. Failure to do so is an error and will have unexpected consequences.

```cpp
class run_on_systemc : public sc_core::sc_module
{
public:

    run_on_systemc()

    void scp_cancel_pendings();
    void scp_fork_on_systemc(std::function<void()> job_entry);
    bool scp_run_on_systemc(std::function<void()> job_entry, bool wait = true);
    bool scp_is_on_systemc() const;
};
```

In order to run a job on the SystemC kernel thread, ***scp_run_on_systemc*** method should be called with an appropriate ```std::function```. The ***scp_run_on_systemc*** takes an optional argument to wait for the job to be scheduled and executed on SystemC, which defaults to true. 
The return status is set to ***true*** if the job has been successfully executed and completed and ***false*** if the job was canceled before it was finished. 

If ***run_on_systemc*** is called from within the SystemC thread, then the job is executed immediately (In this case the return status will be ***true***).

The method ***scp_fork_on_systemc*** is a convenience function that calls ***scp_run_on_systemc*** with wait set to false (there is no return status as the job can not be interrupted).

The method ***scp_cancel_pendings*** cancels all pending jobs. Any jobs which are queue'd waiting to be processed by SystemC will be canceled.


# keep_alive
This is a simple ***sc_module*** convenience class which contains an ***scp_async_event*** initialised to attach to the SystemC kernel, and shall therefore ensure that the simulation does not finish. The module provides a convenience function (***finish()***) to detach the event (and therefore allow the simulation to exit).

```cpp
class keep_alive : public sc_core::sc_module
{
public:
    scp::async_event keep_alive_event;

    keep_alive(sc_core::sc_module_name name): sc_core::sc_module(name) {
        keep_alive_event.async_attach_suspending();
    }
    void finish() {
        keep_alive_event.async_detach_suspending();
        keep_alive_event.notify();
    }
    ~keep_alive() {}
};
```



# realtimelimiter
An instance of this sc_module will ensure that SystemC time never advances more than realtime (with a granularity of 100ms). Convenience functions to `enable` and `disable` this behavior are provided.

```cpp
SC_MODULE (real_time_limiter) {
public:
    void enable();
    void disable();
    real_time_limiter(const sc_core::sc_module_name& name,
                      bool autostart = true,
                      sc_core::sc_time accuracy=sc_core::sc_time(100, sc_core::SC_MS)):
};
```

The constructor, and ***enable*** and ***disable*** funtions must be called from the systemc thread. By default, the ****real_time_limiter*** is constructed to start automatically, with an accuracy of 100ms.
