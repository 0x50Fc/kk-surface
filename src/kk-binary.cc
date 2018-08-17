//
//  kk-binary.cc
//  KKGame
//
//  Created by hailong11 on 2018/7/30.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#include "kk-config.h"
#include "kk-binary.h"

namespace kk {
    
    static Binary * BinaryAllocWithType(BinaryType type) {
        Binary * v = new Binary();
        memset(v,0,sizeof(Binary));
        v->type = type;
        return v;
    };
    
    Binary * BinaryAlloc(duk_context * ctx,duk_idx_t idx) {
        Binary * v = new Binary();
        memset(v,0,sizeof(Binary));
        v->type = BinaryTypeNil;
        
        if(duk_is_function(ctx, idx)) {
            duk_dup(ctx, idx);
            duk_dump_function(ctx);
            v->type = BinaryTypeFunction;
            
            void * b = duk_get_buffer(ctx, -1, & v->functionValue.size);
            
            v->functionValue.data = malloc(v->functionValue.size);
            
            memcpy(v->functionValue.data, b, v->functionValue.size);
            
            duk_pop(ctx);
        } else if(duk_is_buffer_data(ctx, idx)) {
            v->type = BinaryTypeBinary;
            void * s = duk_get_buffer_data(ctx, idx, & v->binaryValue.size);
            v->binaryValue.data = (char *) malloc(v->binaryValue.size);
            memcpy(v->binaryValue.data, s, v->binaryValue.size);
        } else if(duk_is_boolean(ctx, idx)) {
            v->type = BinaryTypeBoolean;
            v->booleanValue = duk_to_boolean(ctx, idx) ? true : false;
        } else if(duk_is_number(ctx, idx)) {
            v->type = BinaryTypeNumber;
            v->doubleValue = duk_to_number(ctx, idx);
        } else if(duk_is_string(ctx, idx)) {
            v->type = BinaryTypeString;
            const char * s = duk_to_lstring(ctx, idx, &v->stringValue.size);
            v->stringValue.data = (char *) malloc(v->stringValue.size + 1);
            memcpy(v->stringValue.data, s, v->stringValue.size + 1);
        } else if(duk_is_array(ctx, idx)) {
            v->type = BinaryTypeArray;
            Binary * p = v;
            
            duk_idx_t i = 0;
            duk_size_t n = duk_get_length(ctx, idx);
            
            while(i < n) {
                duk_get_prop_index(ctx, idx, i);
                p = BinaryAdd(p, BinaryAlloc(ctx, -1));
                duk_pop(ctx);
                i ++;
            }
            
            p->next = BinaryAllocWithType(BinaryTypeEnd);
        } else if(duk_is_object(ctx, idx)) {
            
            v->type = BinaryTypeObject;
            
            Binary * p = v;
            
            duk_enum(ctx, idx, DUK_ENUM_INCLUDE_SYMBOLS);
            
            while(duk_next(ctx, -1, 1)) {
                p = BinaryAdd(p, BinaryAlloc(ctx, -2));
                p = BinaryAdd(p, BinaryAlloc(ctx, -1));
                duk_pop_2(ctx);
            }
            
            duk_pop(ctx);
            
            p->next = BinaryAllocWithType(BinaryTypeEnd);
        }
        
        return v;
    }
    
    Binary * BinaryPushContext(duk_context * ctx,Binary * binary) {
        
        switch (binary->type) {
            case BinaryTypeFunction:
            {
                void * b = duk_push_fixed_buffer(ctx, binary->functionValue.size);
                memcpy(b, binary->functionValue.data, binary->functionValue.size);
                duk_load_function(ctx);
                return binary->next;
            }
                break;
            case BinaryTypeBinary:
            {
                void * b = duk_push_fixed_buffer(ctx, binary->binaryValue.size);
                memcpy(b, binary->binaryValue.data, binary->binaryValue.size);
                duk_push_buffer_object(ctx, -1, 0, binary->binaryValue.size, DUK_BUFOBJ_UINT8ARRAY);
                duk_remove(ctx, -2);
                return binary->next;
            }
                break;
            case BinaryTypeBoolean:
            {
                if(binary->booleanValue) {
                    duk_push_true(ctx);
                } else {
                    duk_push_false(ctx);
                }
                return binary->next;
            }
                break;
            case BinaryTypeNumber:
            {
                duk_push_number(ctx, binary->doubleValue);
                return binary->next;
            }
                break;
            case BinaryTypeString:
            {
                duk_push_lstring(ctx, binary->stringValue.data, binary->stringValue.size);
                return binary->next;
            }
                break;
            case BinaryTypeObject:
            {
                duk_push_object(ctx);
                Binary * p = binary->next;
                while(p && p->type != BinaryTypeEnd) {
                    p = BinaryPushContext(ctx,p);
                    if(p) {
                        p = BinaryPushContext(ctx,p);
                        duk_put_prop(ctx, -3);
                    } else {
                        duk_pop(ctx);
                        break;
                    }
                }
                if(p && p->type == BinaryTypeEnd) {
                    return p->next;
                }
                return p;
            }
                break;
            case BinaryTypeArray:
            {
                duk_push_array(ctx);
                duk_idx_t i = 0;
                Binary * p = binary->next;
                while(p && p->type != BinaryTypeEnd) {
                    duk_push_int(ctx, i);
                    p = BinaryPushContext(ctx,p);
                    duk_put_prop(ctx, -3);
                    i ++;
                }
                if(p->type == BinaryTypeEnd) {
                    return p->next;
                }
                return p;
            }
                break;
            default:
                duk_push_undefined(ctx);
                return binary->next;
                break;
        }
        
    }
    
    void BinaryDealloc(Binary * binary) {
        if(binary->next) {
            BinaryDealloc(binary->next);
        }
        if(binary->type == BinaryTypeBinary) {
            if(binary->binaryValue.data) {
                free(binary->binaryValue.data);
            }
        } else if( binary->type == BinaryTypeString) {
            if(binary->stringValue.data) {
                free(binary->stringValue.data);
            }
        } else if( binary->type == BinaryTypeFunction) {
            if(binary->functionValue.data) {
                free(binary->functionValue.data);
            }
        }
        delete binary;
    }
    
    Binary * BinaryAdd(Binary * p,Binary * binary) {
        while(p && p->next) {
            p = p->next;
        }
        if(p) {
            p->next = binary;
        }
        return binary;
    }
    
}
