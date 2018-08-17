//
//  KKSurfaceContext.h
//  KKSurface
//
//  Created by hailong11 on 2018/8/2.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#import <Foundation/Foundation.h>

#include <KKSurface/kk-config.h>
#include <KKSurface/kk-object.h>
#include <KKSurface/kk-string.h>
#include <KKSurface/kk-binary.h>
#include <KKSurface/kk-block.h>
#include <KKSurface/kk-chan.h>
#include <KKSurface/kk-crypto.h>
#include <KKSurface/kk-dispatch.h>
#include <KKSurface/kk-ev.h>
#include <KKSurface/kk-http.h>
#include <KKSurface/kk-script.h>
#include <KKSurface/kk-wk.h>
#include <KKSurface/kk-ws.h>

typedef void (^KKSurfaceContextBlock)(void);

@interface KKSurfaceContext : NSObject

@property(nonatomic,readonly,assign) kk::DispatchQueue * queue;
@property(nonatomic,readonly,assign) kk::script::Context * jsContext;
@property(nonatomic,readonly,assign) struct event_base * base;
@property(nonatomic,readonly,assign) struct evdns_base * dns;

-(instancetype) initWithName:(NSString *) name queue:(dispatch_queue_t) queue;

-(void) recycle;

-(void) run:(KKSurfaceContextBlock)block;

-(void) openlibs:(duk_context *) newContext queue:(kk::DispatchQueue *) queue;

-(void) openlibs;

+(void) JSContextPushObject:(id) object ctx:(duk_context * )ctx;

+(id) JSContextToObject:(duk_idx_t) idx ctx:(duk_context * )ctx;

+(void) JSContextPushSelector:(id) object selector:(SEL) selector name:(const char *) name ctx:(duk_context * )ctx queue:(dispatch_queue_t) queue;

@end
