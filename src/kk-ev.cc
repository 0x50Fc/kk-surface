//
//  kk-ev.c
//  app
//
//  Created by zhanghailong on 2018/6/8.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#include <pthread.h>

#include "kk-config.h"
#include "kk-ev.h"
#include "kk-script.h"

namespace kk {
    
   
    
    struct evdns_base * ev_dns(duk_context * ctx) {
        
        struct evdns_base * base = nullptr;
        
        duk_get_global_string(ctx, "__evdns_base");
        
        if(duk_is_pointer(ctx, -1)) {
            base = (struct evdns_base *) duk_to_pointer(ctx, -1);
        }
        
        duk_pop(ctx);
        
        return base;
    }
    
    struct event_base * ev_base(duk_context * ctx) {
        
        struct event_base * base = nullptr;
        
        duk_get_global_string(ctx, "__event_base");
        
        if(duk_is_pointer(ctx, -1)) {
            base = (struct event_base *) duk_to_pointer(ctx, -1);
        }
        
        duk_pop(ctx);
        
        return base;
    }
    
    struct Timer {
        duk_context * ctx;
        struct event * event;
        struct timeval tv;
    };
    
    static void ev_Timeout_cb(evutil_socket_t fd, short ev, void * data) {
        
        Timer * v = (Timer *) data;
        duk_context * ctx = v->ctx;
        
        duk_push_global_object(ctx);
        duk_push_sprintf(ctx, "__0x%x",(long) v);
        duk_get_prop(ctx, -2);
        
        if(duk_is_object(ctx, -1)) {
            
            duk_push_string(ctx, "fn");
            duk_get_prop(ctx, -2);
            
            if(duk_is_function(ctx, -1)) {
                
                if(duk_pcall(ctx, 0) != DUK_EXEC_SUCCESS) {
                    kk::script::Error(ctx, -1);
                }
                
            }
            
            duk_pop(ctx);
            
        }
        
        duk_pop_n(ctx,2);
        
        if(v->tv.tv_sec || v->tv.tv_usec) {
            evtimer_add(v->event, &v->tv);
        } else {
            
            duk_push_global_object(ctx);
            duk_push_sprintf(ctx, "__0x%x",(long) v);
            duk_del_prop(ctx, -2);
            duk_pop(ctx);
            
        }
        
    }
    
    static duk_ret_t ev_Timer_dealloc(duk_context *ctx) {
        
        duk_get_prop_string(ctx, -1, "__object");
        
        if(duk_is_pointer(ctx, -1)) {
            
            Timer * v =(Timer *) duk_to_pointer(ctx, -1);
            
            evtimer_del(v->event);
            event_free(v->event);
            
            delete v;
            
        }
        
        return 0;
    }
    
    static duk_ret_t ev_newTimer(duk_context * ctx, void * fn, int tv, int rv) {
        
        event_base * base = ev_base(ctx);
        
        Timer * v = new Timer();
        
        memset(v,0,sizeof(Timer));
        
        v->ctx = ctx;
        v->event = evtimer_new(base, ev_Timeout_cb, v);
        v->tv.tv_sec = rv / 1000;
        v->tv.tv_usec = (rv % 1000) * 1000;
        
        duk_push_global_object(ctx);
        
        duk_push_sprintf(ctx, "__0x%x",(long) v);
        duk_push_object(ctx);
        
        duk_push_string(ctx, "__object");
        duk_push_pointer(ctx, v);
        duk_put_prop(ctx, -3);
        
        duk_push_string(ctx, "fn");
        duk_push_heapptr(ctx, fn);
        duk_put_prop(ctx, -3);
        
        duk_push_c_function(ctx, ev_Timer_dealloc, 1);
        duk_set_finalizer(ctx, -2);
        
        duk_put_prop(ctx, -3);
        
        duk_pop(ctx);
        
        struct timeval tvv = {tv / 1000, (tv % 1000) * 1000};
        
        evtimer_add(v->event, &tvv);
        
        duk_push_sprintf(ctx, "__0x%x",(long) v);
        
        return 1;
        
    }
    
    static duk_ret_t ev_setTimeout(duk_context * ctx) {
        
        int top = duk_get_top(ctx);
        
        if(top > 1 && duk_is_function(ctx, -top) && duk_is_number(ctx, -top +1)) {
            
            void * fn = duk_get_heapptr(ctx, -top);
            int tv = duk_to_int(ctx, -top + 1);
            return ev_newTimer(ctx,fn,tv,0);
        }
        
        return 0;
    }
    
    static duk_ret_t ev_setInterval(duk_context * ctx) {
        
        int top = duk_get_top(ctx);
        
        if(top > 1 && duk_is_function(ctx, -top) && duk_is_number(ctx, -top +1)) {
            void * fn = duk_get_heapptr(ctx, -top);
            int tv = duk_to_int(ctx, -top + 1);
            return ev_newTimer(ctx,fn,tv,tv);
        }
        
        return 0;
    }
    
    static duk_ret_t ev_clear(duk_context * ctx) {
        
        int top = duk_get_top(ctx);
        
        if(top > 0 && duk_is_string(ctx, -top) ) {
            
            duk_push_global_object(ctx);
            duk_dup(ctx, -top -1);
            duk_del_prop(ctx, -2);
            duk_pop(ctx);
            
        }
        
        return 0;
    }
    
    
    void ev_openlibs(duk_context * ctx,event_base * base, evdns_base * dns) {
        
        duk_push_global_object(ctx);
        
        duk_push_string(ctx, "__event_base");
        duk_push_pointer(ctx, base);
        duk_put_prop(ctx, -3);
        
        duk_push_string(ctx, "__evdns_base");
        duk_push_pointer(ctx, dns);
        duk_put_prop(ctx, -3);
        
        duk_push_string(ctx, "setTimeout");
        duk_push_c_function(ctx, ev_setTimeout, 2);
        duk_put_prop(ctx, -3);
        
        duk_push_string(ctx, "clearTimeout");
        duk_push_c_function(ctx, ev_clear, 1);
        duk_put_prop(ctx, -3);
        
        duk_push_string(ctx, "setInterval");
        duk_push_c_function(ctx, ev_setInterval, 2);
        duk_put_prop(ctx, -3);
        
        duk_push_string(ctx, "clearInterval");
        duk_push_c_function(ctx, ev_clear, 1);
        duk_put_prop(ctx, -3);
        
        duk_pop(ctx);
        
    }
    
}
