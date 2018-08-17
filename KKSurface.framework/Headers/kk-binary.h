//
//  kk-binary.h
//  KKGame
//
//  Created by hailong11 on 2018/7/30.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#ifndef kk_binary_h
#define kk_binary_h

#if defined(__APPLE__)

#include <KKSurface/kk-script.h>

#elif defined(__ANDROID_API__)

#include "kk-script.h"

#endif

namespace kk {
    
    enum BinaryType {
        BinaryTypeNil,
        BinaryTypeBinary,
        BinaryTypeObject,
        BinaryTypeArray,
        BinaryTypeString,
        BinaryTypeNumber,
        BinaryTypeBoolean,
        BinaryTypeFunction,
        BinaryTypeEnd
    };
    
    struct Binary {
        BinaryType type;
        union {
            struct {
                void * data;
                size_t size;
            } binaryValue;
            struct {
                char * data;
                size_t size;
            } stringValue;
            double doubleValue;
            bool booleanValue;
            struct {
                void * data;
                size_t size;
            } functionValue;
        };
        struct Binary * next;
    };
    
    Binary * BinaryAlloc(duk_context * ctx,duk_idx_t idx);
    
    Binary * BinaryPushContext(duk_context * ctx,Binary * binary);
    
    Binary * BinaryAdd(Binary * p,Binary * binary);
    
    void BinaryDealloc(Binary * binary);
    
}

#endif /* kk_binary_h */
