//
//  KKSurfaceContext.m
//  KKSurface
//
//  Created by hailong11 on 2018/8/2.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#import "KKSurfaceContext.h"

#define KK_FRAME_SPEED 30


@interface KKSurfaceContext() {
    dispatch_source_t _source;
}

@end

@implementation KKSurfaceContext

@synthesize queue = _queue;
@synthesize jsContext = _jsContext;
@synthesize base = _base;
@synthesize dns = _dns;

-(void) dealloc {
    
    [self recycle];
    
}

-(instancetype) initWithName:(NSString *) name queue:(dispatch_queue_t) queue {
    
    if((self = [super init])) {
        
        if(queue == nil) {
            _queue = new kk::DispatchQueue([name UTF8String]);
            _queue->retain();
        } else {
            _base = event_base_new();
            _queue = new kk::DispatchQueue([name UTF8String],_base);
            
            _source = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0,queue);
            struct event_base * base = _base;
            dispatch_source_set_timer(_source, dispatch_walltime(NULL, 0), (int64_t)(NSEC_PER_SEC / KK_FRAME_SPEED), 0);
            dispatch_source_set_event_handler(_source, ^{
                event_base_loop(base, EVLOOP_NONBLOCK);
            });
            dispatch_resume(_source);
        }
        
        _jsContext = new kk::script::Context();
        _jsContext->retain();
        
        {
            duk_context * ctx = _jsContext->jsContext();
            duk_push_pointer(ctx, (__bridge void *) self);
            duk_put_global_string(ctx, "__KKSurfaceContext");
        }
        
        __weak KKSurfaceContext * v = self;
        
        [self run:^{
            [v openlibs];
        }];
        
    }
    
    return self;
}

-(struct event_base *) base {
    if(_base == nullptr) {
        if(_queue != nullptr) {
            return _queue->base();
        }
    }
    return _base;
}

-(void) recycle {
    
    if(_source) {
        dispatch_source_cancel(_source);
        _source = nil;
    }
    
    if(_queue) {
        _queue->join();
        _queue->release();
        _queue = nullptr;
    }
    
    if(_jsContext) {
        _jsContext->release();
        _jsContext = nullptr;
    }
    
    if(_dns) {
        evdns_base_free(_dns, 0);
        _dns = nullptr;
    }
    
    if(_base) {
        event_base_free(_base);
        _base = nullptr;
    }
    
}

static void KKSurfaceContext_EventOnCreateContext (duk_context * ctx,
                                                   kk::DispatchQueue * queue,
                                                   duk_context * newContext) {
    @autoreleasepool {
        
        KKSurfaceContext * v = nil;
        
        duk_get_global_string(ctx, "__KKSurfaceContext");
        
        if(duk_is_pointer(ctx, -1)) {
            v = (__bridge KKSurfaceContext *) duk_to_pointer(ctx, -1);
        }
        
        duk_pop(ctx);
        
        [v openlibs:newContext queue:queue];
        
    }
    
}

-(void) openlibs:(duk_context *) newContext queue:(kk::DispatchQueue *) queue {
    
    kk::wk_openlibs(newContext, queue, KKSurfaceContext_EventOnCreateContext);
    kk::Crypto_openlibs(newContext);
    
    {
        duk_context * ctx = _jsContext->jsContext();
        duk_push_pointer(ctx, (__bridge void *) self);
        duk_put_global_string(ctx, "__KKSurfaceContext");
    }
    
}

-(void) openlibs {
    
    event_base * base = self.base;
    
    _dns = evdns_base_new(base, EVDNS_BASE_INITIALIZE_NAMESERVERS);
    
    {
        duk_context * ctx = _jsContext->jsContext();
        
        kk::ev_openlibs(ctx, base, _dns);
        kk::wk_openlibs(ctx, _queue, KKSurfaceContext_EventOnCreateContext);
        kk::Crypto_openlibs(ctx);
        
    }
    
}

static void CFBridgingRelease(void * object) {
    if(object != nullptr) {
        CFRelease((CFTypeRef) object);
    }
}

static void KKSurfaceContextRunFunc(kk::DispatchQueue * queue,BK_DEF_ARG) {
    
    BK_GET(fn, void)
    
    @autoreleasepool {
        KKSurfaceContextBlock block = (__bridge KKSurfaceContextBlock) fn;
        block();
    }

}

-(void) run:(KKSurfaceContextBlock)block {
    if(_queue != nullptr && block != nil) {
        BK_CTX
        BK_PTR(fn, CFBridgingRetain(block), CFBridgingRelease);
        _queue->async(KKSurfaceContextRunFunc, BK_ARG);
    }
}


+(void) JSContextPushObject:(id) object ctx:(duk_context * )ctx {
    
    if(object == nil) {
        duk_push_null(ctx);
        return;
    }
    
    if([object isKindOfClass:[NSString class]]) {
        duk_push_string(ctx, [object UTF8String]);
        return;
    }
    
    if([object isKindOfClass:[NSNumber class]]) {
        
        if(strcmp(@encode(BOOL),[object objCType]) == 0) {
            duk_push_boolean(ctx, [object boolValue]);
        } else {
            duk_push_number(ctx, [object doubleValue]);
        }
        return;
    }
    
    if([object isKindOfClass:[NSArray class]]) {
        
        duk_push_array(ctx);
        
        duk_idx_t i = 0;
        
        for(id value in object ){
            
            duk_push_int(ctx, i);
            [self JSContextPushObject:value ctx:ctx];
            duk_put_prop(ctx,-3);
            
        }
        
        return ;
    }
    
    if([object isKindOfClass:[NSDictionary class]]) {
        
        duk_push_object(ctx);
        
        NSEnumerator * keyEnum = [object keyEnumerator];
        NSString * key;
        
        while((key = [keyEnum nextObject])) {
            duk_push_string(ctx, [key UTF8String]);
            [self JSContextPushObject:[object valueForKey:key] ctx:ctx];
            duk_put_prop(ctx, -3);
        }
        
        return;
    }
    
    duk_push_null(ctx);
    
    return;
}

+(id) JSContextToObject:(duk_idx_t) idx ctx:(duk_context * )ctx {
    
    if(duk_is_array(ctx, idx)) {
        
        NSMutableArray * vs = [NSMutableArray arrayWithCapacity:4];
        
        duk_size_t n = duk_get_length(ctx, idx);
        
        for(duk_uarridx_t i =0 ;i<n;i++) {
            duk_get_prop_index(ctx, idx, i);
            id value = [self JSContextToObject: -1 ctx:ctx];
            if(value) {
                [vs addObject:value];
            }
            duk_pop(ctx);
        }
        
        return vs;
    } else if(duk_is_object(ctx, idx)) {
        
        NSMutableDictionary * vs = [NSMutableDictionary dictionaryWithCapacity:4];
        
        duk_enum(ctx, idx, DUK_ENUM_INCLUDE_SYMBOLS);
        
        while(duk_next(ctx, -1, 1)) {
            NSString * key = [NSString stringWithCString:duk_to_string(ctx,-2) encoding:NSUTF8StringEncoding];
            id value = [self JSContextToObject: -1 ctx:ctx];
            if(key && value) {
                [vs setValue:value forKey:key];
            }
            duk_pop_2(ctx);
        }
        
        duk_pop(ctx);
        
        return vs;
        
    } else if(duk_is_boolean(ctx, idx)) {
        return [NSNumber numberWithBool:duk_to_boolean(ctx, idx)];
    } else if(duk_is_string(ctx, idx)) {
        return [NSString stringWithCString:duk_to_string(ctx,idx) encoding:NSUTF8StringEncoding];
    } else if(duk_is_number(ctx, idx)) {
        return [NSNumber numberWithDouble:duk_to_number(ctx, idx)];
    }
    
    return nil;
}

static duk_ret_t KKSurfaceContext_selector(duk_context * ctx) {
    
    @autoreleasepool {
        
        NSMutableArray * arguments = [NSMutableArray arrayWithCapacity:4];
        
        int top = duk_get_top(ctx);
        
        for(int i=0;i<top;i++){
            id v = [KKSurfaceContext JSContextToObject:-top +i ctx:ctx];
            if(v == nil) {
                [arguments addObject:[NSNull null]];
            } else {
                [arguments addObject:v];
            }
        }
        
        id object = nil;
        SEL selector = nil;
        dispatch_queue_t queue = nil;
        
        duk_push_current_thread(ctx);
        
        duk_get_prop_string(ctx, -1, "__object");
        if(duk_is_pointer(ctx, -1)) {
            object = (__bridge id) duk_to_pointer(ctx, -1);
        }
        duk_pop(ctx);
        
        duk_get_prop_string(ctx, -1, "__queue");
        if(duk_is_pointer(ctx, -1)) {
            queue = (__bridge dispatch_queue_t) duk_to_pointer(ctx, -1);
        }
        duk_pop(ctx);
        
        duk_get_prop_string(ctx, -1, "__selector");
        if(duk_is_pointer(ctx, -1)) {
            selector = (SEL) duk_to_pointer(ctx, -1);
        }
        duk_pop(ctx);
        
        if(object && selector && [object respondsToSelector:selector] ) {
            NSMethodSignature *methodSignature = [object methodSignatureForSelector:selector];
            NSInvocation * invocation = [NSInvocation invocationWithMethodSignature:methodSignature];
            [invocation setTarget:object];
            
            NSUInteger i = 1;
            
            for(id v in arguments) {
                id vv = v;
                [invocation setArgument:&vv atIndex:++i];
            }
            
            if(queue) {
                dispatch_async(queue, ^{
                    [invocation invoke];
                });
            } else {
                [invocation invoke];
            }
        }
    }
    
    return 0;
};


+(void) JSContextPushSelector:(id) object selector:(SEL) selector name:(const char *) name ctx:(duk_context * )ctx queue:(dispatch_queue_t) queue {
    
    duk_push_c_function(ctx, KKSurfaceContext_selector, DUK_VARARGS);
    duk_push_pointer(ctx, (__bridge void *) object);
    duk_put_prop_string(ctx, -2, "__object");
    duk_push_pointer(ctx, (__bridge void *) queue);
    duk_put_prop_string(ctx, -2, "__queue");
    duk_push_pointer(ctx, (void *) selector);
    duk_put_prop_string(ctx, -2, "__selector");
    duk_put_prop_string(ctx, -2, name);
    
}


@end
