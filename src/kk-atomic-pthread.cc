//
//  kk-atomic-pthread.cc
//  KKGame
//
//  Created by hailong11 on 2018/8/2.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#include "kk-config.h"
#include "kk-object.h"
#include <pthread.h>
#include <queue>

namespace kk {
    
    class MutexAtomic : public Atomic {
    public:
        
        MutexAtomic() {
            pthread_mutex_init(&_lock, nullptr);
            pthread_mutex_init(&_objectLock, nullptr);
        }
        
        virtual ~MutexAtomic() {
            pthread_mutex_destroy(&_lock);
            pthread_mutex_destroy(&_objectLock);
        }
        
        virtual void lock() {
            pthread_mutex_lock(&_lock);
        }
        
        virtual void unlock() {
            pthread_mutex_unlock(&_lock);
            
            Object * v = nullptr;
            
            do {
                
                pthread_mutex_lock(&_objectLock);
                
                if(_objects.empty()) {
                    v = nullptr;
                } else {
                    v = _objects.front();
                    _objects.pop();
                }
                
                pthread_mutex_unlock(&_objectLock);
                
                if(v != nullptr) {
                    delete v;
                }
                
            } while (v);
        }
        
        virtual void addObject(Object * object) {
            pthread_mutex_lock(&_objectLock);
            _objects.push(object);
            pthread_mutex_unlock(&_objectLock);
        }
        
    private:
        pthread_mutex_t _lock;
        pthread_mutex_t _objectLock;
        std::queue<Object *> _objects;
    };
    
    Atomic * atomic() {
        
        static Atomic * a = nullptr;
        
        if(a == nullptr) {
            a = new MutexAtomic();
        }
        
        return a;
        
    }
    
}
