/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All Rights
 * Reserved.
 * Author: GreenSocs 2022
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#ifndef _SCP_RUNONSYSTEMC_H
#define _SCP_RUNONSYSTEMC_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>

#include <scp/thread_utils/async_event.h>

namespace scp {
class run_on_systemc : public sc_core::sc_module
{
protected:
    class AsyncJob
    {
    public:
        using Ptr = std::shared_ptr<AsyncJob>;

    private:
        std::packaged_task<void()> m_task;

        bool m_cancelled = false;

        void run_job() { m_task(); }

    public:
        AsyncJob(std::function<void()>&& job): m_task(job) {}

        AsyncJob(std::function<void()>& job): m_task(job) {}

        AsyncJob() = delete;
        AsyncJob(const AsyncJob&) = delete;

        void operator()() { run_job(); }

        /**
         * @brief Cancel a job
         *
         * @details Cancel a job by setting m_cancelled to true and by
         * resetting the task. Any waiter will then be unblocked immediately.
         */
        void cancel() {
            m_cancelled = true;
            m_task.reset();
        }

        void wait() {
            auto future = m_task.get_future();

            future.wait();

            if (!m_cancelled) {
                future.get();
            }
        }

        bool is_cancelled() const { return m_cancelled; }
    };

    std::thread::id m_thread_id;

    /* Async job queue */
    std::queue<AsyncJob::Ptr> m_async_jobs;
    AsyncJob::Ptr m_running_job;
    std::mutex m_async_jobs_mutex;

    scp_async_event m_jobs_handler_event;

    // Process inside a thread incase the job calls wait
    void jobs_handler() {
        std::unique_lock<std::mutex> lock(m_async_jobs_mutex);
        for (;;) {
            while (!m_async_jobs.empty()) {
                m_running_job = m_async_jobs.front();
                m_async_jobs.pop();

                lock.unlock();
                sc_core::sc_unsuspendable(); // a wait in the job will cause
                                             // systemc time to advance
                (*m_running_job)();
                sc_core::sc_suspendable();
                lock.lock();

                m_running_job.reset();
            }

            lock.unlock();
            wait(m_jobs_handler_event);
            lock.lock();
        }
    }

    void cancel_pendings_locked() {
        while (!m_async_jobs.empty()) {
            m_async_jobs.front()->cancel();
            m_async_jobs.pop();
        }
    }

    void end_of_simulation() {
        std::lock_guard<std::mutex> lock(m_async_jobs_mutex);

        cancel_pendings_locked();

        if (m_running_job) {
            m_running_job->cancel();
            m_running_job.reset();
        }
    }

public:
    run_on_systemc():
        sc_module(sc_core::sc_module_name("run-on-sysc")),
        m_thread_id(std::this_thread::get_id()),
        m_jobs_handler_event(false) // starve if no more jobs provided
    {
        SC_THREAD(jobs_handler);
    }

    /**
     * @brief Cancel all pending jobs
     *
     * @detail Cancel all the pending jobs. The callers will be unblocked
     *         if they are waiting for the job.
     */
    void scp_cancel_pendings() {
        std::lock_guard<std::mutex> lock(m_async_jobs_mutex);

        cancel_pendings_locked();
    }

    void scp_fork_on_systemc(std::function<void()> job_entry) {
        scp_run_on_systemc(job_entry, false);
    }

    /**
     * @brief Run a job on the SystemC kernel thread
     *
     * @param[in] job_entry The job to run
     * @param[in] wait If true, wait for job completion
     *
     * @return true if the job has been succesfully executed or if `wait`
     *         was false, false if it has been cancelled (see
     *         `run_on_systemc::cancel_all`).
     */
    bool scp_run_on_systemc(std::function<void()> job_entry,
                            bool wait = true) {
        if (scp_is_on_systemc()) {
            job_entry();
            return true;
        } else {
            AsyncJob::Ptr job(new AsyncJob(job_entry));

            {
                std::lock_guard<std::mutex> lock(m_async_jobs_mutex);
                m_async_jobs.push(job);
            }

            m_jobs_handler_event.async_notify();

            if (wait) {
                /* Wait for job completion */
                job->wait();
                return !job->is_cancelled();
            }

            return true;
        }
    }

    /**
     * @return Whether we are on SystemC thread
     */
    bool scp_is_on_systemc() const {
        return std::this_thread::get_id() == m_thread_id;
    }
};
} // namespace scp

#endif // _SCP_RUNONSYSTEMC_H
