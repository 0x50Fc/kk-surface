//
//  kk-block.cpp
//  app
//
//  Created by hailong11 on 2018/7/30.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#include "kk-config.h"
#include "kk-block.h"

namespace kk {
    
    Block::Block(BlockType type,kk::Object * object):_type(type),_object(object),_data(nullptr),_size(0),_dealloc(nullptr) {
        if(type == BlockTypeRetain) {
            if(object) {
                object->retain();
            }
        } else if(type == BlockTypeWeak) {
            if(object) {
                object->weak(&_object);
            }
        } else {
            assert(0);
        }
    }
    
    Block::Block(void * data,size_t size):_type(BlockTypeCopy),_object(nullptr),_size(0),_dealloc(nullptr) {
        _data = malloc(size);
        memcpy(_data, data, size);
    }
    
    Block::Block(void * ptr,BlockPtrDeallocFunc dealloc):_type(BlockTypePtr),_object(nullptr),_size(0),_data(ptr),_dealloc(dealloc) {
        
    }
    
    Block::~Block() {
        if(_type == BlockTypeRetain) {
            if(_object) {
                _object->release();
            }
        } else if(_type == BlockTypeWeak) {
            if(_object) {
                _object->unWeak(&_object);
            }
        } else if(_type == BlockTypeCopy){
            if(_data) {
                free(_data);
            }
        } else if(_type == BlockTypePtr) {
            if(_dealloc) {
                (*_dealloc)(_data);
            }
        }
    }
    
    BlockType Block::type() {
        return _type;
    }
    
    void * Block::ptr() {
        return _data;
    }
    
    void * Block::data(size_t * n) {
        if(n) {
            *n = _size;
        }
        return _data;
    }
    
    kk::Object * Block::object() {
        return _object;
    }
    
    BlockContext::BlockContext() {
        
    }
    
    BlockContext::~BlockContext() {
        
        std::map<std::string,Block *>::iterator i = _blocks.begin();
        
        while(i != _blocks.end()) {
            i->second->release();
            i ++;
        }
        
    }
    
    void BlockContext::add(kk::CString name,Block * block) {
        std::map<std::string,Block *>::iterator i = _blocks.find(name);
        block->retain();
        if(i != _blocks.end()) {
            i->second->release();
        }
        _blocks[name] = block;
    }
    
    void BlockContext::add(kk::CString name,BlockType type,kk::Object * object) {
        add(name, new Block(type,object));
    }
    
    void BlockContext::add(kk::CString name,void * data,size_t size) {
        add(name, new Block(data,size));
    }
    
    void BlockContext::add(kk::CString name,void * ptr,BlockPtrDeallocFunc dealloc) {
        add(name, new Block(ptr,dealloc));
    }
    
    kk::Object * BlockContext::object(kk::CString name) {
        std::map<std::string,Block *>::iterator i = _blocks.find(name);
        if(i != _blocks.end()) {
            return i->second->object();
        }
        return nullptr;
    }
    
    void * BlockContext::data(kk::CString name,size_t * n) {
        std::map<std::string,Block *>::iterator i = _blocks.find(name);
        if(i != _blocks.end()) {
            return i->second->data(n);
        }
        return nullptr;
    }
    
    void * BlockContext::ptr(kk::CString name) {
        std::map<std::string,Block *>::iterator i = _blocks.find(name);
        if(i != _blocks.end()) {
            return i->second->ptr();
        }
        return nullptr;
    }
    
    void * BlockContext::get(kk::CString name) {
        std::map<std::string,Block *>::iterator i = _blocks.find(name);
        if(i != _blocks.end()) {
            Block * b = i->second;
            switch (b->type()) {
                case BlockTypePtr:
                    return b->ptr();
                case BlockTypeCopy:
                    return b->data(nullptr);
                default:
                    return b->object();
            }
            return nullptr;
        }
        return nullptr;
    }
    
}
