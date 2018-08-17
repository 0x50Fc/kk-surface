//
//  kk-wk.cpp
//  KKGame
//
//  Created by hailong11 on 2018/7/30.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#include <pthread.h>
#include "kk-config.h"
#include "kk-wk.h"
#include "kk-script.h"
#include "kk-binary.h"
#include "kk-dispatch.h"

namespace kk {
    
    
    class WebWorker : public kk::script::HeapObject, public kk::script::IObject {
    public:
        WebWorker();
        virtual ~WebWorker();
        
        virtual event_base * base();
        virtual evdns_base * dns();
        virtual kk::script::Context * jsContext();
        virtual duk_context * dukContext();
        
        void open(duk_context * ctx, void * _heapptr, DispatchQueue * main);
        
        virtual duk_ret_t duk_postMessage(duk_context * ctx);
        virtual duk_ret_t duk_terminate(duk_context * ctx);
        DEF_SCRIPT_CLASS
        
        virtual DispatchQueue * main();
        virtual DispatchQueue * queue();
        
        virtual void postMessage(kk::Binary * binary,int top);
        
    protected:
        duk_context * _ctx;
        void * _heapptr;
        kk::Weak _main;
        DispatchQueue * _queue;
        evdns_base * _dns;
        kk::script::Context * _jsContext;
    };
    
    static duk_ret_t WebWorkerAlloc(duk_context * ctx) {
        
        int top = duk_get_top(ctx);
        
        if(top > 0 && duk_is_function(ctx, -top)) {
            
            duk_dup(ctx, -top);
            duk_dump_function(ctx);
            duk_size_t n;
            void * data = duk_get_buffer(ctx, -1, &n);
            
            {
                EventOnCreateContext onCreate = nullptr;
                DispatchQueue * queue = nullptr;
                WebWorker * v = new WebWorker();
                
                duk_context * jsContext = v->dukContext();
                
                duk_push_global_object(ctx);
                duk_push_string(ctx, "__EventOnCreateContext");
                duk_get_prop(ctx, -2);
                
                if(duk_is_pointer(ctx, -1)) {
                    onCreate = (EventOnCreateContext) duk_to_pointer(ctx, -1);
                }
                
                duk_pop(ctx);
                
                duk_push_global_object(ctx);
                duk_push_string(ctx, "__queue");
                duk_get_prop(ctx, -2);
                
                if(duk_is_pointer(ctx, -1)) {
                    queue = (DispatchQueue *) duk_to_pointer(ctx, -1);
                }
                
                duk_pop_2(ctx);
                
                kk::ev_openlibs(jsContext, v->base(), v->dns());
                kk::wk_openlibs(jsContext, v->queue(), onCreate);
                
                if(onCreate) {
                    (*onCreate)(ctx,queue,jsContext);
                }
                
                void * b = duk_push_fixed_buffer(jsContext, n);
                b = memcpy(b, data, n);
                duk_load_function(jsContext);
                
                duk_pop(ctx);
                
                kk::script::PushObject(ctx, v);
                
                v->open(ctx,duk_get_heapptr(ctx, -1), queue);
                
                return 1;
            }
            
        }
        
        return 0;
    }
    
    static void WebWorkerPostMessageDispatchFunc(DispatchQueue * queue,BK_DEF_ARG) {
        BK_GET(v, WebWorker)
        BK_GET(binary,kk::Binary)
        BK_GET(top,int)
        
        if(v != nullptr) {
            v->postMessage(binary,*top);
        }
    }
    
    void WebWorker::postMessage(kk::Binary * binary,int top) {
        
        if(_ctx != nullptr) {
            
            kk::Binary * p = binary;
            
            duk_push_heapptr(_ctx, _heapptr);
            
            if(duk_is_object(_ctx, -1)) {
                
                duk_push_string(_ctx, "onmessage");
                duk_get_prop(_ctx, -2);
                
                if(duk_is_function(_ctx, -1)) {
                    
                    duk_dup(_ctx, -2);
                    
                    while(p) {
                        p = kk::BinaryPushContext(_ctx, p);
                    }
                    
                    if(duk_pcall_method(_ctx, top) != DUK_EXEC_SUCCESS) {
                        kk::script::Error(_ctx, -1);
                    }
                    
                }
                
                duk_pop(_ctx);
                
            }
            
            duk_pop(_ctx);
            
        }
        
    }
    
    static duk_ret_t WebWorkerPostMessage(duk_context * ctx) {
        
        kk::Binary * binary = nullptr;
        kk::Binary * p = nullptr;
        int top = duk_get_top(ctx);
        
        for(int i=0;i<top;i++) {
            if(p == nullptr) {
                p = kk::BinaryAlloc(ctx, -top + i);
                binary = p;
            } else {
                p = kk::BinaryAdd(p, kk::BinaryAlloc(ctx, -top + i));
            }
        }
        
        if(binary == nullptr) {
            return 0;
        }
        
        BK_CTX
        BK_PTR(binary, binary, kk::BinaryDealloc)
        BK_COPY(top, top)
        
        WebWorker * v = nullptr;
        
        duk_push_current_function(ctx);
        duk_push_string(ctx, "__object");
        duk_get_prop(ctx, -2);
        
        if(duk_is_pointer(ctx, -1)) {
            v = (WebWorker *) duk_to_pointer(ctx, -1);
        }
        
        duk_pop_2(ctx);
        
        if(v) {
            
            BK_WEAK(v, v)
            
            DispatchQueue * main = v->main();
            
            if(main) {
                main->async(WebWorkerPostMessageDispatchFunc, BK_ARG);
            }
        }
        
        
        return 0;
    }
    
    IMP_SCRIPT_CLASS_BEGIN(nullptr, WebWorker, WebWorker)
    
    static kk::script::Method methods[] = {
        {"postMessage",(kk::script::Function) &WebWorker::duk_postMessage},
        {"terminate",(kk::script::Function) &WebWorker::duk_terminate},
    };
    
    kk::script::SetMethod(ctx, -1, methods, sizeof(methods) / sizeof(kk::script::Method));
    
    
    duk_push_string(ctx, "alloc");
    duk_push_c_function(ctx, WebWorkerAlloc, 1);
    duk_put_prop(ctx, -3);
    
    IMP_SCRIPT_CLASS_END
    
    
    WebWorker::WebWorker():_jsContext(nullptr),_dns(nullptr),_ctx(nullptr),_heapptr(nullptr),_main(nullptr){
        _queue = new DispatchQueue("kk::WebWorker");
        _queue->retain();
        _dns = evdns_base_new(_queue->base(), EVDNS_BASE_INITIALIZE_NAMESERVERS);
        _jsContext = new kk::script::Context();
        _jsContext->retain();
    }
    
    WebWorker::~WebWorker() {
        _queue->join();
        _jsContext->release();
        if(_dns) {
            evdns_base_free(_dns, 0);
        }
        _queue->release();
    }
    
    DispatchQueue * WebWorker::main() {
        return _main.as<DispatchQueue>();
    }
    
    DispatchQueue * WebWorker::queue() {
        return _queue;
    }
    
    event_base * WebWorker::base() {
        return _queue->base();
    }
    
    evdns_base * WebWorker::dns() {
        return _dns;
    }
    
    kk::script::Context * WebWorker::jsContext() {
        return _jsContext;
    }
    
    duk_context * WebWorker::dukContext() {
        if(_jsContext) {
            return _jsContext->jsContext();
        }
        return nullptr;
    }
    
    static void WebWorkerOpenFunc(DispatchQueue * queue,BK_DEF_ARG) {
        
        BK_GET(v, WebWorker)
        
        if(v != nullptr) {
            
            duk_context * ctx = v->dukContext();
            
            duk_push_global_object(ctx);
            
            duk_push_string(ctx, "postMessage");
            duk_push_c_function(ctx, WebWorkerPostMessage, DUK_VARARGS);
            
            duk_push_string(ctx, "__object");
            duk_push_pointer(ctx, (void *) v);
            duk_put_prop(ctx, -3);
            
            duk_put_prop(ctx, -3);
            
            duk_pop(ctx);
            
            if(duk_pcall(ctx, 0) != DUK_EXEC_SUCCESS) {
                kk::script::Error(ctx, -1);
            }
            
            duk_pop(ctx);
        }
        
    }
    
    void WebWorker::open(duk_context * ctx, void * heapptr, DispatchQueue * main) {
        _ctx = ctx;
        _heapptr = heapptr;
        _main = main;
        BK_CTX
        BK_WEAK(v,this)
        _queue->async(WebWorkerOpenFunc, BK_ARG);
    }
    
    static void WebWorkerPostMessageFunc(DispatchQueue * queue,BK_DEF_ARG) {
        
        BK_GET(v, WebWorker)
        BK_GET(binary, kk::Binary)
        BK_GET(top,int)
        
        kk::Binary * p = binary;
        
        if(v != nullptr) {
            
            duk_context * ctx = v->dukContext();
            
            duk_push_global_object(ctx);
            
            duk_push_string(ctx, "onmessage");
            duk_get_prop(ctx, -2);
            
            if(duk_is_function(ctx, -1)) {
                
                while(p) {
                    p = kk::BinaryPushContext(ctx, p);
                }
                
                if(duk_pcall(ctx, * top) != DUK_EXEC_SUCCESS) {
                    kk::script::Error(ctx, -1);
                }
                
            }
            
            duk_pop_2(ctx);
        }
        
    }
    
    duk_ret_t WebWorker::duk_postMessage(duk_context * ctx) {
        
        kk::Binary * binary = nullptr;
        kk::Binary * p = nullptr;
        int top = duk_get_top(ctx);
        
        for(int i=0;i<top;i++) {
            if(p == nullptr) {
                p = kk::BinaryAlloc(ctx, -top + i);
                binary = p;
            } else {
                p = kk::BinaryAdd(p, kk::BinaryAlloc(ctx, -top + i));
            }
        }
        
        if(binary == nullptr) {
            return 0;
        }
        
        BK_CTX
        BK_PTR(binary, binary, kk::BinaryDealloc)
        BK_WEAK(v,this)
        BK_COPY(top,top)
        
        _queue->async(WebWorkerPostMessageFunc, BK_ARG);

        return 0;
    }
    
    duk_ret_t WebWorker::duk_terminate(duk_context * ctx) {
        _queue->loopbreak();
        return 0;
    }
    
    void wk_openlibs(duk_context * ctx,kk::DispatchQueue * queue,EventOnCreateContext onCreateContext) {
        
        duk_push_global_object(ctx);
        
        duk_push_string(ctx, "__EventOnCreateContext");
        duk_push_pointer(ctx, (void *) onCreateContext);
        duk_put_prop(ctx, -3);
        
        duk_push_string(ctx, "__queue");
        duk_push_pointer(ctx, (void *) queue);
        duk_put_prop(ctx, -3);
        
        duk_pop(ctx);
        
        kk::script::SetPrototype(ctx, &WebWorker::ScriptClass);
        
    }
    
}

