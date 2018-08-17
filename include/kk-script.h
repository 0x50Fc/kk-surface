//
//  kk-script.h
//  KKGame
//
//  Created by zhanghailong on 2018/2/5.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#ifndef kk_script_h
#define kk_script_h

#if defined(__APPLE__)

#include <KKDuktape/KKDuktape.h>
#include <KKSurface/kk-object.h>

#elif defined(__ANDROID_API__)

#include "duktape.h"
#include "kk-object.h"

#endif

namespace kk {
    
    extern kk::Float Kernel;
    
    namespace script {
        
        typedef duk_ret_t (kk::Object::*Function)(duk_context * jsContext);
        
        typedef void (*ClassPrototypeFunc)(duk_context * jsContext);
        
        struct Class {
            Class * isa;
            CString name;
            ClassPrototypeFunc prototype;
            duk_c_function alloc;
        };
        
        class Context : public kk::Object {
        public:
            Context();
            virtual ~Context();
            virtual duk_context * jsContext();
            virtual kk::Object * object(kk::CString key);
            virtual void setObject(kk::CString key,kk::Object * object);
        private:
            duk_context * _jsContext;
            std::map<kk::String,kk::Object *> _objects;
        };
        
        class IHeapObject {
        public:
            virtual void setHeapptr(void * heapptr,duk_context * ctx) = 0;
            virtual void * heapptr(duk_context * ctx) = 0;
            virtual void removeHeapptr(duk_context * ctx) = 0;
        };
        
        class HeapObject: public kk::Object, public IHeapObject {
        public:
            HeapObject();
            virtual ~HeapObject();
            virtual void setHeapptr(void * heapptr,duk_context * ctx);
            virtual void * heapptr(duk_context * ctx);
            virtual void removeHeapptr(duk_context * ctx);
        protected:
            std::map<duk_context *,void *> _heapptrs;
        };
        
        class Object : public kk::Object {
        public:
            Object(Context * context,duk_idx_t idx);
            virtual ~Object();
            virtual Context * context();
            virtual duk_context * jsContext();
            virtual void * heapptr();
        private:
            Weak _context;
            void * _heapptr;
        };
        
        class IObject  {
        public:
            virtual Class * getScriptClass() = 0;
        };
        
        Context * GetContext(duk_context * jsContext);
        
        void PushObject(duk_context * ctx,kk::Object * object);
        
        void InitObject(duk_context * ctx,duk_idx_t idx,kk::Object * object);
        
        kk::Object * GetObject(duk_context * ctx,duk_idx_t idx);
        
        bool ObjectEqual(Object * a, Object * b);
        
        void SetPrototype(duk_context * ctx,Class * isa);
        
        bool GetPrototype(duk_context * ctx,Class * isa);
        
        bool GetPrototype(duk_context * ctx,kk::Object * object);
        
        void Error(duk_context * ctx, duk_idx_t idx);
        
        void Error(duk_context * ctx, duk_idx_t idx,kk::CString prefix);
        
        struct Property {
            CString name;
            Function getter;
            Function setter;
        };
        
        void SetProperty(duk_context * ctx, duk_idx_t idx, Property * propertys, kk::Uint count);
        
        struct Method {
            CString name;
            Function invoke;
        };
        
        void SetMethod(duk_context * ctx, duk_idx_t idx, Method * methods, kk::Uint count);
        
        void openlibs(duk_context * ctx, kk::CString basePath);
        
        void exec(duk_context * ctx, kk::CString path, kk::CString name);
        
#define DEF_SCRIPT_CLASS \
    public: \
        static kk::script::Class ScriptClass; \
        static void ScriptClassPrototype(duk_context * ctx); \
        static duk_ret_t ScriptObjectAlloc(duk_context * ctx); \
        virtual kk::script::Class * getScriptClass();


#define IMP_SCRIPT_CLASS_BEGIN(isa,object,name) \
kk::script::Class object::ScriptClass = {((kk::script::Class *) isa),(#name),(kk::script::ClassPrototypeFunc)&object::ScriptClassPrototype,(duk_c_function) &object::ScriptObjectAlloc} ;\
duk_ret_t object::ScriptObjectAlloc(duk_context * ctx) { \
    kk::script::PushObject(ctx, new object()); \
    return 1; \
} \
kk::script::Class * object::getScriptClass() { return &object::ScriptClass; }  \
void object::ScriptClassPrototype(duk_context * ctx) {
    
#define IMP_SCRIPT_CLASS_END \
}
        

    }
    
   
}

#endif /* kk_script_h */
