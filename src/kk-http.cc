//
//  kk-http.cpp
//  app
//
//  Created by hailong11 on 2018/6/11.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#include "kk-config.h"
#include "kk-http.h"
#include "kk-string.h"
#include "kk-ev.h"

namespace kk {
    
    enum HttpBodyType {
        HttpBodyTypeString,HttpBodyTypeBytes,HttpBodyTypeJSON
    };
    
    class HttpTask : public kk::Object ,public kk::script::IObject {
    public:
        virtual void dealloc();
        virtual void cancel();
        virtual void open(kk::Uint64 id, Http * http ,evhttp_request * req,HttpBodyType bodyType,evhttp_cmd_type method, const char * url,int timeout_in_secs);
        virtual kk::Uint64 id();
        
        virtual duk_ret_t duk_cancel(duk_context * ctx);
        
        kk::Strong onload;
        kk::Strong onfail;
        
        DEF_SCRIPT_CLASS
    protected:
        kk::String _host;
        kk::Uint64 _id;
        Weak _http;
        evhttp_connection * _conn;
        evhttp_request * _req;
        HttpBodyType _bodyType;
        
        virtual void onFail(kk::CString errmsg);
        virtual void onLoad();
        
        virtual void remove();
        
        friend void HttpTask_error_cb(enum evhttp_request_error err, void * data);
        friend void HttpRequest_cb(struct evhttp_request * req, void * data);
    };
    
    IMP_SCRIPT_CLASS_BEGIN(nullptr, HttpTask, HttpTask)
    
    static kk::script::Method methods[] = {
        {"cancel",(kk::script::Function) &HttpTask::duk_cancel}
    };
    
    kk::script::SetMethod(ctx, -1, methods, sizeof(methods) / sizeof(kk::script::Method));
    
    duk_push_string(ctx, "alloc");
    duk_del_prop(ctx, -2);
    
    IMP_SCRIPT_CLASS_END
    
    
    kk::Uint64 HttpTask::id() {
        return _id;
    }
    
    void HttpTask::dealloc() {
        if(_req != nullptr) {
            evhttp_cancel_request(_req);
            _req = nullptr;
            _conn = nullptr;
        }
    }
    
    void HttpTask::remove() {
        
        {
            Http * http = _http.as<Http>();
            if(http) {
                http->remove(this);
            }
        }
        
        _req = nullptr;
        _conn = nullptr;
        
    }
    
    void HttpTask::cancel() {
        
        if(_req != nullptr) {
            
            evhttp_cancel_request(_req);

            if(_conn) {
                evhttp_connection_free(_conn);
                _conn = nullptr;
            }
            
            remove();
            
        }
        
    }
    
    void HttpTask_error_cb(enum evhttp_request_error err, void * data) {
        HttpTask * httpTask = (HttpTask *) data;
        if(err == EVREQ_HTTP_TIMEOUT) {
            httpTask->onFail("请求超时!");
        } else if(err ==EVREQ_HTTP_EOF || err == EVREQ_HTTP_REQUEST_CANCEL) {
            
        } else {
            httpTask->onFail("请求错误");
        }
    }
    
    void HttpTask::open(kk::Uint64 id,Http * http ,evhttp_request * req,HttpBodyType bodyType,evhttp_cmd_type method, const char * url,int timeout_in_secs) {
        
        _id = id;
        _http = http;
        _req = req;
        _bodyType = bodyType;
        
        evhttp_uri * uri = evhttp_uri_parse(url);
        
        if(uri == nullptr || evhttp_uri_get_host(uri) == nullptr) {
            onFail("错误的URL");
            return;
        }
        
        _host = evhttp_uri_get_host(uri);
        
        char fmt[128];
        
        if(evhttp_uri_get_port(uri) != -1) {
            _host.append(":");
            snprintf(fmt, sizeof(fmt), "%d",evhttp_uri_get_port(uri));
            _host.append(fmt);
        }
        
        int port = evhttp_uri_get_port(uri);
        
        if(port == -1) {
            port = 80;
        }
        
        struct evkeyvalq * headers = evhttp_request_get_output_headers(req);
        
        evhttp_add_header(headers, "Host", _host.c_str());
        
        _conn = evhttp_connection_base_new(http->base(), http->dns(), evhttp_uri_get_host(uri), port);
    
        evhttp_connection_set_max_body_size(_conn, INT32_MAX);
        evhttp_connection_set_timeout(_conn, timeout_in_secs);
        evhttp_connection_free_on_completion(_conn);
        
        evhttp_request_set_error_cb(_req, HttpTask_error_cb);
        
        evhttp_make_request(_conn, req, method, url);
        
        this->retain();
    
        evhttp_uri_free(uri);
    }
    
    void HttpTask::onFail(kk::CString errmsg) {
        
        {
            kk::script::Object * fn = onfail.as<kk::script::Object>();
            
            if(fn && fn->jsContext()) {
                
                duk_context * ctx = fn->jsContext();
                
                duk_push_heapptr(ctx, fn->heapptr());
                
                if(duk_is_function(ctx, -1)) {
                    
                    duk_push_string(ctx, errmsg);
                    
                    if(duk_pcall(ctx, 1) != DUK_EXEC_SUCCESS) {
                        kk::script::Error(ctx, -1);
                    }
                    
                }
                
                duk_pop_n(ctx, 1);
            }
            
        }
        
        remove();
    }
    
    void HttpTask::onLoad() {
        
        if(evhttp_request_get_response_code(_req) == 200) {
        
            evbuffer * input = evhttp_request_get_input_buffer(_req);
            
            void * data = EVBUFFER_DATA(input);
            size_t n = EVBUFFER_LENGTH(input);
            
            {
                
                kk::script::Object * fn = onload.as<kk::script::Object>();
                
                if(fn && fn->jsContext()) {
                    
                    duk_context * ctx = fn->jsContext();

                    duk_push_heapptr(ctx, fn->heapptr());
                    
                    if(duk_is_function(ctx, -1)) {
                        
                        if(_bodyType == HttpBodyTypeJSON) {
                            duk_push_lstring(ctx, (const char *) data, n);
                            duk_json_decode(ctx, -1);
                        } else if(_bodyType == HttpBodyTypeBytes) {
                            void * d = duk_push_fixed_buffer(ctx, n);
                            memcpy(d, data, n);
                            duk_push_buffer_object(ctx, -1, 0, n, DUK_BUFOBJ_UINT8ARRAY);
                            duk_remove(ctx, -2);
                            
                        } else {
                            duk_push_lstring(ctx, (const char *) data, n);
                        }
                        
                        if(duk_pcall(ctx, 1) != DUK_EXEC_SUCCESS) {
                            kk::script::Error(ctx, -1);
                        }
                        
                    }
                    
                    duk_pop_n(ctx, 1);
                    
                }
                
            }
            
            remove();
            
        } else {
            onFail(evhttp_request_get_response_code_line(_req));
        }
        
    }
    
    duk_ret_t HttpTask::duk_cancel(duk_context * ctx) {
        cancel();
        return 0;
    }
    
    IMP_SCRIPT_CLASS_BEGIN(nullptr, Http, Http)
    
    static kk::script::Method methods[] = {
        {"send",(kk::script::Function) &Http::duk_send}
    };
    
    kk::script::SetMethod(ctx, -1, methods, sizeof(methods) / sizeof(kk::script::Method));
    
    duk_push_string(ctx, "alloc");
    duk_del_prop(ctx, -2);
    
    IMP_SCRIPT_CLASS_END
    
    Http::Http():Http(nullptr,nullptr) {
    }
    
    Http::Http(struct event_base * base,struct evdns_base * dns):_base(base),_dns(dns),_autoId(0) {
        
    }
    
    Http::~Http() {
        
        {
            std::map<kk::Uint64,HttpTask *>::iterator i = _tasks.begin();
            
            while(i != _tasks.end()) {
                
                HttpTask * task = i->second;
                task->dealloc();
                task->release();
                i ++;
            }
        }
        
    }
    
    void HttpRequest_cb(struct evhttp_request * req, void * data) {
        HttpTask * httpTask = (HttpTask *) data;
        httpTask->onLoad();
        httpTask->release();
    }
    
    duk_ret_t Http::duk_send(duk_context * ctx) {
        
        int top = duk_get_top(ctx);
        
        if(top >0 && duk_is_object(ctx, -top)) {
            
            HttpBodyType bodyType = HttpBodyTypeString;
            kk::String url;
            
            HttpTask * httpTask = new HttpTask();
        
            evhttp_request * req = evhttp_request_new(HttpRequest_cb, httpTask);
            evhttp_cmd_type method = EVHTTP_REQ_GET;
            int timeout_in_secs = 10;
            
            {
                duk_get_prop_string(ctx, -top, "method");
                
                if(duk_is_string(ctx, -1)) {
                    if(kk::CStringEqual(duk_to_string(ctx, -1), "POST")) {
                        method = EVHTTP_REQ_POST;
                    }
                }
                
                duk_pop(ctx);
            }
            
            {
                duk_get_prop_string(ctx, -top, "timeout");
                
                if(duk_is_number(ctx, -1)) {
                    timeout_in_secs = duk_to_int(ctx, -1);
                    if(timeout_in_secs < 0) {
                        timeout_in_secs = 10;
                    }
                }
                
                duk_pop(ctx);
            }
            
            {
                duk_get_prop_string(ctx, -top, "type");
                
                if(duk_is_string(ctx, -1)) {
                    kk::CString v = duk_to_string(ctx, -1);
                    if(kk::CStringEqual(v, "json")) {
                        bodyType = HttpBodyTypeJSON;
                    } else if(kk::CStringEqual(v, "bytes")) {
                        bodyType = HttpBodyTypeBytes;
                    }
                }
                
                duk_pop(ctx);
            }
            
            {
                duk_get_prop_string(ctx, -top, "url");
                
                if(duk_is_string(ctx, -1)) {
                    url = duk_to_string(ctx, -1);
                }
                
                duk_pop(ctx);
            }
            
            {
                duk_get_prop_string(ctx, -top, "headers");
                
                if(duk_is_object(ctx, -1)) {
                    
                    struct evkeyvalq * headers = evhttp_request_get_output_headers(req);
                    
                    duk_enum(ctx, -1, DUK_ENUM_INCLUDE_SYMBOLS);
                    
                    while(duk_next(ctx, -1, 1)) {
                        
                        kk::CString key = duk_to_string(ctx, -2);
                        kk::CString value = duk_to_string(ctx, -1);
                        
                        if(key && value) {
                            evhttp_add_header(headers, key, value);
                        }
            
                    }
                    
                    duk_pop(ctx);
                    
                }
                
                duk_pop(ctx);
            }
            
            if(method == EVHTTP_REQ_GET) {
                
                duk_get_prop_string(ctx, -top, "data");
                
                if(duk_is_object(ctx, -1)) {
                    
                    std::string::size_type i = url.find("?");
                    
                    if(i == std::string::npos) {
                        url.append("?");
                    } else if(i != url.size() -1) {
                        url.append("&");
                    }
                    
                    i = 0;
                    
                    duk_enum(ctx, -1, DUK_ENUM_INCLUDE_SYMBOLS);
                    
                    while(duk_next(ctx, -1, 1)) {
                        
                        kk::CString key = duk_to_string(ctx, -2);
                        
                        if(i != 0) {
                            url.append("&");
                        }
                        
                        url.append(key);
                        url.append("=");
                        
                        duk_get_global_string(ctx, "encodeURIComponent");
                        duk_dup(ctx, -2);
                        duk_call(ctx, 1);
                        
                        url.append(duk_to_string(ctx, -1));
                        
                        duk_pop_n(ctx,3);
                        
                        i ++;
                    }
                    
                    duk_pop(ctx);
                    
                }
                
                duk_pop(ctx);
                
            } else if(method == EVHTTP_REQ_POST) {
                
                duk_get_prop_string(ctx, -top, "data");
                
                if(duk_is_object(ctx, -1)) {
                    
                    kk::String data;
                    
                    size_t i = 0;
                    
                    duk_enum(ctx, -1, DUK_ENUM_INCLUDE_SYMBOLS);
                    
                    while(duk_next(ctx, -1, 1)) {
                        
                        kk::CString key = duk_to_string(ctx, -2);
                        
                        if(i != 0) {
                            data.append("&");
                        }
                        
                        data.append(key);
                        data.append("=");
                        
                        duk_get_global_string(ctx, "encodeURIComponent");
                        duk_dup(ctx, -2);
                        duk_call(ctx, 1);
                        
                        data.append(duk_to_string(ctx, -1));
                        
                        duk_pop_n(ctx,3);
                        
                        i ++;
                    }
                    
                    duk_pop(ctx);
                    
                    struct evkeyvalq * headers = evhttp_request_get_output_headers(req);
                    evhttp_add_header(headers, "Content-Type", "application/x-www-form-urlencoded");
                    
                    evbuffer * output = evhttp_request_get_output_buffer(req);
                    evbuffer_add(output, data.c_str(), data.size());
                    
                } else if(duk_is_string(ctx, -1)) {
                    size_t n;
                    kk::CString data = duk_to_lstring(ctx, -1, &n);
                    evbuffer * output = evhttp_request_get_output_buffer(req);
                    evbuffer_add(output, data, n);
                } else if(duk_is_buffer_data(ctx, -1)) {
                    duk_size_t n;
                    void * bytes = duk_get_buffer_data(ctx, -1, &n);
                    evbuffer * output = evhttp_request_get_output_buffer(req);
                    evbuffer_add(output, bytes, n);
                }
                
                duk_pop(ctx);
                
            }
            
            kk::Uint64 id = ++_autoId;
        
            kk::script::PushObject(ctx, httpTask);
            
            duk_get_prop_string(ctx, -top -1, "onload");
            
            if(duk_is_function(ctx, -1)) {
                httpTask->onload = new kk::script::Object(kk::script::GetContext(ctx),-1);
            }
            
            duk_pop(ctx);
            
            duk_get_prop_string(ctx, -top -1, "onfail");
            
            if(duk_is_function(ctx, -1)) {
                httpTask->onfail = new kk::script::Object(kk::script::GetContext(ctx),-1);
            }
            
            duk_pop(ctx);
            
            httpTask->retain();
            _tasks[id] = httpTask;
            
            httpTask->open(id,this, req, bodyType, method, url.c_str(),timeout_in_secs);
            
 
            return 1;
            
        }
        
        
        return 0;
    }
    
    void Http::remove(HttpTask * httpTask) {
        kk::Uint64 id = httpTask->id();
        std::map<kk::Uint64,HttpTask *>::iterator i = _tasks.find(id);
        if(i != _tasks.end()) {
            HttpTask * v = i->second;
            v->release();
            _tasks.erase(i);
        }
    }
    
    struct event_base * Http::base(){
        return _base;
    }
    
    struct evdns_base * Http::dns() {
        return _dns;
    }
    
}
