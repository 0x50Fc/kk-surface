//
//  kk-dispatch.h
//  app
//
//  Created by hailong11 on 2018/7/30.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#ifndef kk_dispatch_h
#define kk_dispatch_h


#if defined(__APPLE__)

#include <KKSurface/kk-object.h>
#include <KKSurface/kk-chan.h>
#include <KKSurface/kk-block.h>
#include <KKSurface/kk-ev.h>

#elif defined(__ANDROID_API__)

#include "kk-object.h"
#include "kk-chan.h"
#include "kk-block.h"
#include "kk-ev.h"

#endif

#include <pthread.h>
#include <typeinfo>

namespace kk {
    
    class DispatchQueue;
    
    typedef void (*DispatchFunc)(DispatchQueue * queue,BK_DEF_ARG);
    
    class DispatchQueue : public kk::Object {
    public:
        DispatchQueue(kk::CString name);
        DispatchQueue(kk::CString name,event_base * base);
        virtual ~DispatchQueue();
        virtual struct event_base * base();
        virtual void async(DispatchFunc func, BK_DEF_ARG);
        virtual void sync(DispatchFunc func, BK_DEF_ARG);
        virtual void loopbreak();
        virtual void join();
        virtual kk::CString name();
    protected:
        virtual void run();
        kk::Chan * _chan;
        struct event_base * _base;
        struct event * _event;
        bool _attach;
        bool _joined;
        bool _loopbreak;
        pthread_t _pid;
        kk::String _name;
        friend void DispatchQueueCB(evutil_socket_t fd, short ev, void * ctx);
        friend void * DispatchQueueRun(void * data);
    };
    
}



#endif /* kk_dispatch_h */
