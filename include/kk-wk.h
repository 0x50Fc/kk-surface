//
//  kk-wk.h
//  KKGame
//
//  Created by hailong11 on 2018/7/30.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#ifndef kk_wk_h
#define kk_wk_h


#if defined(__APPLE__)

#include <KKSurface/kk-ev.h>
#include <KKSurface/kk-dispatch.h>

#elif defined(__ANDROID_API__)

#include "kk-ev.h"
#include "kk-dispatch.h"

#endif


namespace kk {
    
    typedef void (*EventOnCreateContext) (duk_context * ctx, kk::DispatchQueue * queue, duk_context * newContext);
    
    void wk_openlibs(duk_context * ctx,kk::DispatchQueue * queue, EventOnCreateContext onCreateContext);
    
}

#endif /* kk_wk_h */
