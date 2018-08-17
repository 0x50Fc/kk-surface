//
//  KKSurfaceCGContext.h
//  KKSurface
//
//  Created by hailong11 on 2018/8/2.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <KKDuktape/KKDuktape.h>

@interface KKSurfaceCGContext : NSObject

+(void) openlibs:(duk_context *) ctx;

+(void) pushCGContext:(CGContextRef) CGContext ctx:(duk_context *) ctx;

@end
