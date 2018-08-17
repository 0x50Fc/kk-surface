
#include "kk-config.h"
#include "kk-object.h"
#include <list>
#include <strstream>
#include <typeinfo>

#ifdef __ANDROID_API__

#include <android/log.h>

#endif

namespace kk {
    
    Ref::Ref():_object(nullptr) {
        
    }
    
    Object * Ref::get() {
        return _object;
    }

    Strong::Strong():Ref() {
        
    }
    
    Strong::Strong(Object * object):Ref() {
        set(object);
    }
    
    Strong::Strong(const Strong &ref):Ref() {
        set(ref._object);
    }
    
    Strong::~Strong() {
        if(this->_object) {
            this->_object->release();
        }
    }
    
    void Strong::set(Object * object) {
        if(this->_object != object) {
            if(object) {
                object->retain();
            }
            if(this->_object) {
                this->_object->release();
            }
            this->_object = object;
        }
    }
    
    Strong& Strong::operator=(Object * object) {
        set(object);
        return * this;
    }
    
    Strong& Strong::operator=(Ref& ref) {
        set(ref.get());
        return * this;
    }
    
    Strong& Strong::operator=(Strong& ref) {
        set(ref.get());
        return * this;
    }
    
    Weak::Weak():Ref() {
        
    }
    
    Weak::Weak(Object * object):Ref() {
        set(object);
    }
    
    Weak::Weak(const Weak & ref):Ref() {
        set(ref._object);
    }
    
    Weak::~Weak() {
        if(this->_object) {
            this->_object->unWeak(&this->_object);
        }
    }
    
    void Weak::set(Object * object) {
        if(_object != object) {
            if(object) {
                object->weak(&this->_object);
            }
            if(_object) {
                _object->unWeak(&this->_object);
            }
            _object = object;
        }
    }
    
    Weak& Weak::operator=(Object * object) {
        set(object);
        return * this;
    }
    
    Weak& Weak::operator=(Ref& ref) {
        set(ref.get());
        return * this;
    }
    
    Weak& Weak::operator=(Weak& ref) {
        set(ref.get());
        return * this;
    }
    
    Object::Object(): _retainCount(0) {
    }
    
    Object::~Object(){
        
        Atomic * a = atomic();
        
        if(a != nullptr) {
            a->lock();
        }
        
        std::set<Object **>::iterator i =_weakObjects.begin();
        
        while(i != _weakObjects.end()) {
            Object ** v = * i;
            if(v) {
                *v = NULL;
            }
            i ++;
        }
        
        if(a != nullptr) {
            a->unlock();
        }
        
    }
    
    std::string Object::toString() {
        return std::string(typeid(Object).name());
    }
    
    void Object::release() {
        Atomic * a = atomic();
        if(a != nullptr) {
            a->lock();
        }
        _retainCount --;
        if(_retainCount == 0) {
            if(a != nullptr) {
                a->addObject(this);
            } else {
                delete this;
            }
        }
        if(a != nullptr) {
            a->unlock();
        }
    }
    
    void Object::retain() {
        Atomic * a = atomic();
        if(a != nullptr) {
            a->lock();
        }
        _retainCount ++;
        if(a != nullptr) {
            a->unlock();
        }
    }
    
    int Object::retainCount() {
        return _retainCount;
    }
    
    void Object::weak(Object ** ptr) {
        Atomic * a = atomic();
        if(a != nullptr) {
            a->lock();
        }
        _weakObjects.insert(ptr);
        if(a != nullptr) {
            a->unlock();
        }
    }
    
    void Object::unWeak(Object ** ptr) {
        Atomic * a = atomic();
        if(a != nullptr) {
            a->lock();
        }
        std::set<Object **>::iterator i = _weakObjects.find(ptr);
        if(i != _weakObjects.end()) {
            _weakObjects.erase(i);
        }
        if(a != nullptr) {
            a->unlock();
        }
    }
    
#ifdef __ANDROID_API__

    void LogV(const char * format, va_list va) {
        __android_log_vprint(ANDROID_LOG_DEBUG,"kk",format,va);
    }

    void Log(const char * format, ...) {
        va_list va;
        va_start(va, format);
        LogV(format, va);
        va_end(va);
    }

#else

    void LogV(const char * format, va_list va) {

        time_t now = time(NULL);
        
        struct tm * p = localtime(&now);
        
        printf("[KK] [%04d-%02d-%02d %02d:%02d:%02d] ",1900 + p->tm_year,p->tm_mon + 1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
        vprintf(format, va);
        printf("\n");
    
    }
    
    void Log(const char * format, ...) {
        va_list va;
        va_start(va, format);
        LogV(format, va);
        va_end(va);
    }
#endif
    
}
