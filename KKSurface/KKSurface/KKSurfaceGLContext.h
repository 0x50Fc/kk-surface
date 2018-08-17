//
//  KKSurfaceGLContext.h
//  KKSurface
//
//  Created by hailong11 on 2018/8/2.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <KKDuktape/KKDuktape.h>

@interface KKSurfaceGLContext : NSObject

+(void) openlibs:(duk_context *) ctx;

@end
