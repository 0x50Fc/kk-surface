//
//  kk-block.h
//  app
//
//  Created by hailong11 on 2018/7/30.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#ifndef kk_block_h
#define kk_block_h

#if defined(__APPLE__)

#include <KKSurface/kk-object.h>

#elif defined(__ANDROID_API__)

#include "kk-object.h"

#endif

namespace kk {
    
    enum BlockType {
        BlockTypePtr,BlockTypeCopy,BlockTypeRetain,BlockTypeWeak
    };
    
    typedef void (* BlockPtrDeallocFunc)(void * ptr);
    
    class Block : public kk::Object {
    public:
        Block(BlockType type,kk::Object * object);
        Block(void * data,size_t size);
        Block(void * ptr,BlockPtrDeallocFunc dealloc);
        virtual ~Block();
        virtual void * ptr();
        virtual void * data(size_t * n);
        virtual kk::Object * object();
        virtual BlockType type();
    protected:
        BlockType _type;
        kk::Object * _object;
        void * _data;
        size_t _size;
        BlockPtrDeallocFunc _dealloc;
    };
    
    
    class BlockContext : public kk::Object {
    public:
        BlockContext();
        virtual ~BlockContext();
        virtual void add(kk::CString name,Block * block);
        virtual void add(kk::CString name,BlockType type,kk::Object * object);
        virtual void add(kk::CString name,void * data,size_t size);
        virtual void add(kk::CString name,void * ptr,BlockPtrDeallocFunc dealloc);
        virtual kk::Object * object(kk::CString name);
        virtual void * data(kk::CString name,size_t * n);
        virtual void * ptr(kk::CString name);
        virtual void * get(kk::CString name);
    protected:
        std::map<std::string,Block *> _blocks;
    };
    
#define BK_CTX kk::Strong __BK_CTX = new kk::BlockContext();\
    
#define BK_RETAIN(name,object) __BK_CTX.as<kk::BlockContext>()->add(#name,kk::BlockTypeRetain,(kk::Object *) (object));\

#define BK_WEAK(name,object) __BK_CTX.as<kk::BlockContext>()->add(#name,kk::BlockTypeWeak,(kk::Object *) (object));\

#define BK_COPY(name,data) __BK_CTX.as<kk::BlockContext>()->add(#name,(void *) &(data),sizeof(data));\

#define BK_PTR(name,ptr,dealloc) __BK_CTX.as<kk::BlockContext>()->add(#name,(void *) (ptr),(kk::BlockPtrDeallocFunc)(dealloc));\

#define BK_GET(name,type) type * name = (type *) __BK_CTX->get(#name);\

#define BK_ARG (__BK_CTX.as<kk::BlockContext>())
    
#define BK_DEF_ARG BlockContext * __BK_CTX
    
}

#endif /* kk_block_h */
