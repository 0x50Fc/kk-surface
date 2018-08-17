//
//  KKSurfaceCGContext.m
//  KKSurface
//
//  Created by hailong11 on 2018/8/2.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#import "KKSurfaceCGContext.h"

@implementation KKSurfaceCGContext

+(void) openlibs:(duk_context *) ctx {
    
    duk_push_object(ctx);
    

    duk_put_global_string(ctx, "__CGContext");
}

+(void) pushCGContext:(CGContextRef) CGContext ctx:(duk_context *) ctx {
    
    duk_push_object(ctx);
    
    duk_push_pointer(ctx, CGContext);
    duk_put_prop_string(ctx, -2, "__CGContext");
    
    duk_get_global_string(ctx, "__CGContext");
    duk_set_prototype(ctx, -2);
    
}

@end
