//
//  kk-crypto.h
//  KKGame
//
//  Created by zhanghailong on 2018/2/7.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#ifndef kk_crypto_h
#define kk_crypto_h

#if defined(__APPLE__)

#include <KKSurface/kk-script.h>

#elif defined(__ANDROID_API__)

#include "kk-script.h"

#endif

namespace kk {
    
    String Crypto_MD5(CString string);
    
    void Crypto_openlibs(duk_context * ctx);
    
}

#endif /* kk_crypto_h */
