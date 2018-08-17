//
//  kk-ev.h
//  app
//
//  Created by zhanghailong on 2018/6/8.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#ifndef kk_ev_h
#define kk_ev_h


#if defined(__APPLE__)

#include <KKEvent/KKEvent.h>
#include <KKSurface/kk-object.h>
#include <KKSurface/kk-script.h>

#elif defined(__ANDROID_API__)

#include "kk-object.h"
#include "kk-script.h"

#include <event.h>
#include <evhttp.h>
#include <evdns.h>

#endif

namespace kk {
    
    struct event_base * ev_base(duk_context * ctx);
    struct evdns_base * ev_dns(duk_context * ctx);
    
    void ev_openlibs(duk_context * ctx,struct event_base * base, struct evdns_base * dns);
    
}

#endif /* kk_event_hpp */
