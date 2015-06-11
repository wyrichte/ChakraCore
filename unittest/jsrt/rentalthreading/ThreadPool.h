//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#pragma once

namespace JsrtUnitTests
{     
    class ThreadPool
    {   
    public:
        ThreadPool::ThreadPool(DWORD minThreads, DWORD maxThreads)
        {
            if (minThreads > maxThreads)
            {
                VERIFY_FAIL(L"ThreadPool: Bad arguments constructing");
            }

            this->pool = CreateThreadpool(NULL);
            if (this->pool == NULL)
            {
                VERIFY_FAIL(L"ThreadPool: error creating thread pool");
            }

            SetThreadpoolThreadMaximum(this->pool, maxThreads);
            if (!SetThreadpoolThreadMinimum(this->pool, minThreads))
            {
                VERIFY_FAIL(L"ThreadPool: error configuring thread pool");
            }
        }   

        ThreadPool::~ThreadPool()
        {
            CloseThreadpool(this->pool);
        }
        
        template <class T>
        void QueueWorkItem(void (T::* function)(), T * _this)
        {
            if (function == NULL)
            {
                VERIFY_FAIL(L"ThreadPool: NULL callback");
            }

            WorkItem<T> * item = new WorkItem<T>(function, _this);
            item->_this = _this;
            item->function = function;

            if (!QueueUserWorkItem(WorkItemCallbackInternal<T>, item, WT_EXECUTEDEFAULT))
            {
                VERIFY_FAIL(L"ThreadPool: Failed to enqueue work item");
            }
        }

    private:
        template <class T>
        struct WorkItem
        {
            WorkItem(void (T::* function)(), T * _this)
                : function(function), _this(_this)
            {
            }

            T * _this;
            void (T::* function)();
        };

        template <class T>
        static DWORD WINAPI WorkItemCallbackInternal(void * workItem)
        {            
            WorkItem<T> * asWorkItem = reinterpret_cast<WorkItem<T> *>(workItem);
            try
            {                
                (*(asWorkItem->_this).*(asWorkItem->function))();
                delete asWorkItem;
            }
            catch (...)
            {
                VERIFY_FAIL(L"Exception from queued item callback");
            }            

            return 0;
        }

    private:
        PTP_POOL pool;
    };
}