/******************************************************************************
          .d8888b.   .d8888b.  
         d88P  Y88b d88P  Y88b 
         888    888        888 
88888b.  888    888      .d88P 
888 "88b 888    888  .od888P"  
888  888 888    888 d88P"      
888  888 Y88b  d88P 888"       
888  888  "Y8888P"  888888888              Open Kaillera Arcade Netplay Library
*******************************************************************************
Copyright (c) Open Kaillera Team 2003-2008

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice must be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#include "PosixThread.h"

namespace n02 {
    void* nPThreaadCCNV posix_thread_proc (PosixThread* thread)
    {
        thread->running = true;
        thread->run();
        thread->running = false;
        return 0;
    }
};

#ifdef N02_WIN32

/* Windows implementation */

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <windows.h>

namespace n02 {

    PosixThread::PosixThread(bool captureCurrent)
    {
        waitable = handle = 0;
        running = false;
        priority = PTHREAD_PRIORITY_NORMAL;

        if (captureCurrent) {
            handle = GetCurrentThread();
            priority = GetThreadPriority((HANDLE)handle);
        }
    }

    PosixThread::~PosixThread()
    {

    }


    int PosixThread::start()
    {
        handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)posix_thread_proc, this, 0, 0);
        if (handle) {
            return 0;
        } else {
            return GetLastError(); 
        }
    }

    int PosixThread::stop()
    {
        DWORD exiT;
        GetExitCodeThread((HANDLE)handle, &exiT);
        running = false;
        waitable = 0;
        return (TerminateThread((HANDLE)handle, exiT)!=0)? 0: GetLastError();
    }

    bool PosixThread::isRunning()
    {
        return running;
    }

    bool PosixThread::isWaiting()
    {
        return waitable != 0;
    }

    int PosixThread::prioritize(int prio)
    {
        SetThreadPriority((HANDLE)handle, priority=prio);
        return prio;
    }

    bool PosixThread::wait(int timeout)
    {
        waitable = CreateEvent(0, FALSE, FALSE, 0);
        int ret = WaitForSingleObject ((HANDLE)waitable, timeout);
        CloseHandle((HANDLE)waitable);
        waitable = 0;
        return ret==0;
    }

    void PosixThread::notify()
    {
        if (waitable)
            SetEvent((HANDLE)waitable);
    }

    void PosixThread::yield()
    {
//        SwitchToThread();
    }

    void PosixThread::sleep(int ms)
    {
        Sleep(ms);
    }

    int PosixThread::getCurrentThreadId() {
        return GetCurrentThreadId();
    }

};

#else

#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <errno.h>

namespace n02 {

    PosixThread::PosixThread(bool captureCurrent)
    {
        waitable = handle = 0;
        running = false;
        priority = PTHREAD_PRIORITY_NORMAL;

        if (captureCurrent) {
            handle = reinterpret_cast<void*>(pthread_self());
            //priority = GetThreadPriority((HANDLE)handle);
        }
    }

    PosixThread::~PosixThread()
    {

    }


    int PosixThread::start()
    {
        pthread_t handlee;
        if (pthread_create (&handlee, 0, reinterpret_cast<void* (*)(void*)>(posix_thread_proc), this)) {
            pthread_detach (handlee);
            handle = reinterpret_cast<void*>(handlee);
            return 0;
        } else {
            return -1; 
        }
    }

    int PosixThread::stop()
    {
        pthread_cancel ((pthread_t)handle);
        running = false;
        return 0;
    }

    bool PosixThread::isRunning()
    {
        return running;
    }

    bool PosixThread::isWaiting()
    {
        return waitable != 0;
    }

    int PosixThread::prioritize(int prio)
    {
        //SetThreadPriority((HANDLE)handle, priority=prio);
        return prio;
    }

    // Stolen  the wait and notify functions from juce
    // cbf wasting time on it since its already done
struct EventStruct
{
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    bool triggered;
};

    bool PosixThread::wait(int timeout)
    {

	EventStruct* const es = new EventStruct();
	es->triggered = false;
	
	pthread_cond_init (&es->condition, 0);
	pthread_mutex_init (&es->mutex, 0);
	
	waitable = es;
	
	bool ok = true;
	pthread_mutex_lock (&es->mutex);
	
	if (! es->triggered)
	{
		if (timeout < 0)
		{
		pthread_cond_wait (&es->condition, &es->mutex);
		}
		else
		{
		struct timespec time;
	
	#if N02_MAC
		time.tv_sec = timeout / 1000;
		time.tv_nsec = (timeout % 1000) * 1000000;
		pthread_cond_timedwait_relative_np (&es->condition, &es->mutex, &time);
	#else
		struct timeval t;
		int timeout = 0;
	
		gettimeofday (&t, 0);
	
		time.tv_sec  = t.tv_sec  + (timeout / 1000);
		time.tv_nsec = (t.tv_usec + ((timeout % 1000) * 1000)) * 1000;
	
		while (time.tv_nsec >= 1000000000)
		{
			time.tv_nsec -= 1000000000;
			time.tv_sec++;
		}
	
		while (! timeout)
		{
			timeout = pthread_cond_timedwait (&es->condition, &es->mutex, &time);
	
			if (! timeout)
			// Success
			break;
	
			if (timeout == EINTR)
			// Go round again
			timeout = 0;
		}
	#endif
		}
	
		ok = es->triggered;
	}

	waitable = 0;
	
	es->triggered = false;
	
	pthread_mutex_unlock (&es->mutex);
	
	pthread_cond_destroy (&es->condition);
	pthread_mutex_destroy (&es->mutex);
	
	delete es;
	
	return ok;
	
    }

    void PosixThread::notify()
    {
        if (waitable) {
		EventStruct* const es = (EventStruct*) waitable;
		pthread_mutex_lock (&es->mutex);
		es->triggered = true;
		pthread_cond_broadcast (&es->condition);
		pthread_mutex_unlock (&es->mutex);
	}
    }

    void PosixThread::yield()
    {
        pthread_yield();
    }

    void PosixThread::sleep(int ms)
    {
        usleep(ms * 1000);
    }

    int PosixThread::getCurrentThreadId() {
        return static_cast<int>(pthread_self());
    }

};

#endif
