//
//  kk-object.h
//
//  Created by 张海龙 on 2018/02/01
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#ifndef _KK_OBJECT_H
#define _KK_OBJECT_H

#include <string>
#include <vector>
#include <set>
#include <map>

namespace kk {
    
    typedef int Int;
    typedef unsigned char Uint8;
    typedef unsigned Uint;
    typedef long long Int64;
    typedef unsigned long long Uint64;
    typedef bool Boolean;
    typedef double Double;
    typedef std::string String;
    typedef const char * CString;
    typedef long Intptr;
    typedef float Float;
    
    class Object;
    
    class Atomic {
    public:
        virtual void lock() = 0;
        virtual void unlock() = 0;
        virtual void addObject(Object * object) = 0;
    };
    
    extern Atomic * atomic();
    
    class Object {
        
    private:
        int _retainCount;
        std::set<Object **> _weakObjects;
    public:
        
        Object();
        
        virtual ~Object();
        
        virtual String toString();
        
        virtual void release();
        
        virtual void retain();
        
        virtual int retainCount();
        
        virtual void weak(Object ** ptr);
        
        virtual void unWeak(Object ** ptr);
    
    };
    
    class Ref {
    public:
        Ref();
        virtual Object * get();
        virtual void set(Object * object) = 0;
        template<class T>
        T * as() { return dynamic_cast<T *>(get());}
    protected:
        Object * _object;
    };
    
    class Strong : public Ref {
    public:
        Strong();
        Strong(Object * object);
        Strong(const Strong &ref);
        virtual ~Strong();
        virtual void set(Object * object);
        Strong& operator=(Object * object);
        Strong& operator=(Ref& ref);
        Strong& operator=(Strong& ref);
    };
    
    class Weak : public Ref {
    public:
        Weak();
        Weak(Object * object);
        Weak(const Weak & ref);
        virtual ~Weak();
        virtual void set(Object * object);
        Weak& operator=(Object * object);
        Weak& operator=(Ref& ref);
        Weak& operator=(Weak& ref);
    };
    
    void LogV(const char * format, va_list va);
    
    void Log(const char * format, ...);
}

#endif
