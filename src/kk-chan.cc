//
//  kk-chan.cpp
//  KKGame
//
//  Created by hailong11 on 2018/7/30.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#include "kk-config.h"
#include "kk-chan.h"

namespace kk {
    
    
    void ChanObjectRelease(Chan * chan,ChanObject object) {
        kk::Object * v = (kk::Object *) object;
        if(v) {
            v->release();
        }
    }
    
    Chan::Chan():Chan(nullptr) {
        
    }
    
    Chan::Chan(ChanObjectReleaseFunc release):_release(release) {
        pthread_mutex_init(&_lock, nullptr);
    }
    
    Chan::~Chan() {
        
        pthread_mutex_lock(&_lock);
        
        while(!_queue.empty()) {
            
            ChanObject v = _queue.front();
            
            if(_release) {
                (*_release)(this,v);
            }
            
            _queue.pop();
            
        }
        
        pthread_mutex_unlock(&_lock);
        
    }
    
    void Chan::push(ChanObject object) {
        pthread_mutex_lock(&_lock);
        _queue.push(object);
        pthread_mutex_unlock(&_lock);
    }
    
    ChanObject Chan::pop() {
        
        ChanObject v = nullptr;
        
        pthread_mutex_lock(&_lock);
        
        if(!_queue.empty()) {
            v = _queue.front();
            _queue.pop();
        }
        
        pthread_mutex_unlock(&_lock);
        
        return v;
    }
    
    void Chan::releaseObject(ChanObject object) {
        if(_release) {
            (*_release)(this,object);
        }
    }
    
}
