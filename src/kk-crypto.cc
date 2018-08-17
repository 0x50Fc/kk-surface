//
//  kk-crypto.cpp
//  KKGame
//
//  Created by zhanghailong on 2018/2/7.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#include "kk-config.h"
#include "kk-crypto.h"

#if defined(__APPLE__)

#include <CommonCrypto/CommonCrypto.h>

#elif defined(__ANDROID_API__)

#include "md5.h"

#else

#include <openssl/md5.h>

#endif

namespace kk {
    
#if defined(__APPLE__)
    
    String Crypto_MD5(CString string) {
        
        CC_MD5_CTX m;
        
        CC_MD5_Init(&m);
        
        if(string) {
            CC_MD5_Update(&m, string, (CC_LONG) strlen(string));
        }
        
        unsigned char md[16];
        
        CC_MD5_Final(md, &m);
        
        char s[40] = "";
        
        snprintf(s, sizeof(s), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
                 ,md[0],md[1],md[2],md[3],md[4],md[5],md[6],md[7]
                 ,md[8],md[9],md[10],md[11],md[12],md[13],md[14],md[15]);
        
        return s;
    }
#elif defined(__ANDROID_API__)

    String Crypto_MD5(CString string) {

        md5_state_t m;

        md5_init(&m);

        if(string) {
            md5_append(&m, (md5_byte_t *) string, (size_t) strlen(string));
        }

        md5_byte_t md[16];

        md5_finish(&m,md);

        char s[40] = "";

        snprintf(s, sizeof(s), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
                ,md[0],md[1],md[2],md[3],md[4],md[5],md[6],md[7]
                ,md[8],md[9],md[10],md[11],md[12],md[13],md[14],md[15]);

        return s;
    }

#else
   
    String Crypto_MD5(CString string) {
        
        MD5_CTX m;
        
        MD5_Init(&m);
        
        if(string) {
            MD5_Update(&m, string, (size_t) strlen(string));
        }
        
        unsigned char md[16];
        
        MD5_Final(md, &m);
        
        char s[40] = "";
        
        snprintf(s, sizeof(s), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
                 ,md[0],md[1],md[2],md[3],md[4],md[5],md[6],md[7]
                 ,md[8],md[9],md[10],md[11],md[12],md[13],md[14],md[15]);
        
        return s;
    }

#endif
    
    static duk_ret_t Crypto_MD5_func(duk_context * ctx) {
        int top = duk_get_top(ctx);
        
        if(top >0) {
            if(duk_is_string(ctx, -top)) {
                String v = Crypto_MD5(duk_to_string(ctx, -top));
                duk_push_string(ctx, v.c_str());
                return 1;
            }
        }
        
        return 0;
    }
    
    void Crypto_openlibs(duk_context * ctx) {
        
        duk_push_global_object(ctx);
        
        duk_push_object(ctx);
        
        duk_push_c_function(ctx, Crypto_MD5_func, 1);
        duk_put_prop_string(ctx, -2, "md5");
        
        duk_put_prop_string(ctx, -2, "crypto");
        
        duk_pop(ctx);
    }
}
