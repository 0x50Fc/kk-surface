//
//  KKSurfaceElement.m
//  KKSurface
//
//  Created by hailong11 on 2018/8/2.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#import "KKSurfaceElement.h"


@interface KKSurfaceElement() {
    KKSurfaceContext * _context;
}

@end

@implementation KKSurfaceElement

-(void) dealloc {

    [self uninstallSurface];
    
}

-(NSString *) reuse {
    return [self get:@"reuse"];
}

-(void) installSurface {
    
    UIView * view = self.view;
    
    if(view == nil) {
        return;
    }

    if(_context != nil) {
        return;
    }
    
    NSString * path = [self get:@"path"];
    
    if(path == nil) {
        return;
    }
    
    NSString * queue = [self get:@"queue"];
    
    if(queue == nil || [queue isEqualToString:@"main"]) {
        _context = [[KKSurfaceContext alloc] initWithName:@"KKSurfaceElement" queue:dispatch_get_main_queue()];
    } else {
        _context = [[KKSurfaceContext alloc] initWithName:@"KKSurfaceElement" queue:nil];
    }
    
    __weak KKSurfaceElement * element = self;
    
    NSMutableDictionary * attrs = [NSMutableDictionary dictionaryWithCapacity:4];
    
    for(NSString * key in [self keys]) {
        attrs[key] = [self get:key];
    }
    
    CGRect frame = self.frame;
    CGSize contentSize = self.contentSize;
    
    NSString * basePath = [self.viewContext basePath];
    
    if(basePath == nil) {
        basePath = @"";
    }
    
    [_context run:^{
        
        KKSurfaceElement * e = element;
        
        if(e) {
            
            KKSurfaceContext * context = e.context;
            
            duk_context * ctx = context.jsContext->jsContext();
            
            kk::script::openlibs(ctx, [[basePath stringByAppendingString:path] UTF8String]);
            
            duk_push_object(ctx);
            
            [KKSurfaceContext JSContextPushSelector:e selector:@selector(emit:data:) name:"emit" ctx:ctx queue:dispatch_get_main_queue()];
            
            [KKSurfaceContext JSContextPushObject:attrs ctx:ctx];
            duk_put_prop_string(ctx, -2, "attrs");
            
            duk_push_number(ctx, frame.origin.x);
            duk_put_prop_string(ctx, -2, "x");
            duk_push_number(ctx, frame.origin.y);
            duk_put_prop_string(ctx, -2, "y");
            duk_push_number(ctx, frame.size.width);
            duk_put_prop_string(ctx, -2, "width");
            duk_push_number(ctx, frame.size.height);
            duk_put_prop_string(ctx, -2, "height");
            duk_push_number(ctx, contentSize.width);
            duk_put_prop_string(ctx, -2, "contentWidth");
            duk_push_number(ctx, contentSize.height);
            duk_put_prop_string(ctx, -2, "contentHeight");
            duk_push_number(ctx, KKPixelUnitRPX());
            duk_put_prop_string(ctx, -2, "rpx");
            duk_push_number(ctx, KKPixelUnitPX());
            duk_put_prop_string(ctx, -2, "px");
            duk_push_number(ctx, KKPixelUnitVW());
            duk_put_prop_string(ctx, -2, "vw");
            duk_push_number(ctx, KKPixelUnitVH());
            duk_put_prop_string(ctx, -2, "vh");
            
            duk_put_global_string(ctx, "surface");
            
            NSString * main = [path stringByAppendingPathComponent:@"main.js"];
            
            kk::script::exec(ctx, [[basePath stringByAppendingPathComponent:main] UTF8String], [main UTF8String]);
            
        }
    }];
    
    [self willInstallSurfaceContext];
    
    [_context run:^{
        
        KKSurfaceElement * e = element;
        
        if(e) {
            
            KKSurfaceContext * context = e.context;
            
            duk_context * ctx = context.jsContext->jsContext();

            NSString * main = [path stringByAppendingPathComponent:@"main.js"];
            
            kk::script::exec(ctx, [[basePath stringByAppendingPathComponent:main] UTF8String], [main UTF8String]);
            
        }
    }];
    
    [self didInstallSurfaceContext];
}

-(void) didLayouted {
    [super didLayouted];
    
    if(_context) {
        
        CGRect frame = self.frame;
        CGSize contentSize = self.contentSize;
        
        __weak KKSurfaceElement * element = self;
        
        [_context run:^{
            
            KKSurfaceElement * e = element;
            
            if(e) {
                
                KKSurfaceContext * context = e.context;
                
                duk_context * ctx = context.jsContext->jsContext();
                
                duk_get_global_string(ctx, "surface");
                
                if(duk_is_object(ctx, -1)) {
                    
                    duk_push_number(ctx, frame.origin.x);
                    duk_put_prop_string(ctx, -2, "x");
                    duk_push_number(ctx, frame.origin.y);
                    duk_put_prop_string(ctx, -2, "y");
                    duk_push_number(ctx, frame.size.width);
                    duk_put_prop_string(ctx, -2, "width");
                    duk_push_number(ctx, frame.size.height);
                    duk_put_prop_string(ctx, -2, "height");
                    duk_push_number(ctx, contentSize.width);
                    duk_put_prop_string(ctx, -2, "contentWidth");
                    duk_push_number(ctx, contentSize.height);
                    duk_put_prop_string(ctx, -2, "contentHeight");
                    
                    duk_get_prop_string(ctx, -1, "onresize");
                    
                    if(duk_is_function(ctx, -1)) {
                        
                        duk_push_number(ctx, frame.size.width);
                        duk_put_prop_string(ctx, -2, "width");
                        duk_push_number(ctx, frame.size.height);
                        duk_put_prop_string(ctx, -2, "height");
                        
                        if(duk_pcall(ctx, 2) != DUK_EXEC_SUCCESS) {
                            kk::script::Error(ctx, -1);
                        }
                        
                    }
                    
                    duk_pop(ctx);
                    
                }
                
                duk_pop(ctx);
                
            }
            
        }];
         
    }
}

-(void) emit:(NSString *)name data:(NSDictionary *) data {
    KKElementEvent * e = [[KKElementEvent alloc] initWithElement:self];
    e.data = data;
    [super emit:name event:e];
}

-(void) changedKey:(NSString *)key {
    [super changedKey:key];
    
    NSString * value = [self get:key];
    
    __weak KKSurfaceElement * element = self;
    
    [_context run:^{
        
        KKSurfaceElement * e = element;
        
        if(e) {
            
            KKSurfaceContext * context = e.context;
            
            duk_context * ctx = context.jsContext->jsContext();
            
            duk_get_global_string(ctx, "surface");
            
            if(duk_is_object(ctx, -1)) {
                
                duk_get_prop_string(ctx, -1, "onchange");
                
                if(duk_is_function(ctx, -1)) {
                    
                    duk_push_string(ctx, [key UTF8String]);
                    
                    if(value == nil) {
                        duk_push_null(ctx);
                    } else {
                        duk_push_string(ctx, [value UTF8String]);
                    }
                    
                    if(duk_pcall(ctx, 2) != DUK_EXEC_SUCCESS) {
                        kk::script::Error(ctx, -1);
                    }
                    
                }
                
                duk_pop(ctx);
                
            }
            
            duk_pop(ctx);
            
        }
        
    }];
    
}

-(void) emit:(NSString *)name event:(KKEvent *)event {
    [super emit:name event:event];
    
    if([event isKindOfClass:[KKElementEvent class]]) {
        
        id data = [(KKElementEvent *) event data];
        
        __weak KKSurfaceElement * element = self;
        
        [_context run:^{
            
            KKSurfaceElement * e = element;
            
            if(e) {
                
                KKSurfaceContext * context = e.context;
                
                duk_context * ctx = context.jsContext->jsContext();
                
                duk_get_global_string(ctx, "surface");
                
                if(duk_is_object(ctx, -1)) {
                    
                    duk_get_prop_string(ctx, -1, "on");
                    
                    if(duk_is_function(ctx, -1)) {
                        
                        duk_push_string(ctx, [name UTF8String]);
                        [KKSurfaceContext JSContextPushObject:data ctx:ctx];
                        
                        if(duk_pcall(ctx, 2) != DUK_EXEC_SUCCESS) {
                            kk::script::Error(ctx, -1);
                        }
                        
                    }
                    
                    duk_pop(ctx);
                    
                }
                
                duk_pop(ctx);
                
                
            }
            
        }];
        
    }
}

-(void) uninstallSurface {
    
    if(_context != nil) {
        
        [self willUninstallSurfaceContext];
        
        [_context recycle];
        _context = nil;
    }
    
}

-(void) setView:(UIView *)view {
    [self uninstallSurface];
    [super setView:view];
    if(view) {
        [self installSurface];
    }
}

-(void) didInstallSurfaceContext {
    
}

-(void) willUninstallSurfaceContext {
    
}

-(void) willInstallSurfaceContext {
    
}

@end
