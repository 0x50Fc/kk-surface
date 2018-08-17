//
//  kk-ws.cc
//  app
//
//  Created by hailong11 on 2018/6/8.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#include "kk-config.h"
#include "kk-ws.h"
#include "kk-ev.h"

#ifndef ntohll

#define ntohll(x) \
((__uint64_t)((((__uint64_t)(x) & 0xff00000000000000ULL) >> 56) | \
(((__uint64_t)(x) & 0x00ff000000000000ULL) >> 40) | \
(((__uint64_t)(x) & 0x0000ff0000000000ULL) >> 24) | \
(((__uint64_t)(x) & 0x000000ff00000000ULL) >>  8) | \
(((__uint64_t)(x) & 0x00000000ff000000ULL) <<  8) | \
(((__uint64_t)(x) & 0x0000000000ff0000ULL) << 24) | \
(((__uint64_t)(x) & 0x000000000000ff00ULL) << 40) | \
(((__uint64_t)(x) & 0x00000000000000ffULL) << 56)))

#define htonll(x) ntohll(x)

#endif

#define Sec_WebSocket_Key "RCfYMqhgCo4N4E+cIZ0iPg=="
#define MAX_BUF_SIZE 2147483647

#define KKFinMask           0x80
#define KKOpCodeMask        0x0F
#define KKRSVMask           0x70
#define KKMaskMask          0x80
#define KKPayloadLenMask    0x7F
#define KKMaxFrameSize      32


namespace kk {
    

    enum WebSocketOpCode {
        WebSocketOpCodeContinueFrame = 0x0,
        WebSocketOpCodeTextFrame = 0x1,
        WebSocketOpCodeBinaryFrame = 0x2,
        WebSocketOpCodeConnectionClose = 0x8,
        WebSocketOpCodePing = 0x9,
        WebSocketOpCodePong = 0xA,
    };
    
    static duk_ret_t WebSocketAlloc(duk_context * ctx) {
        int top = duk_get_top(ctx);
        
        kk::CString url = nullptr;
        kk::CString protocol = nullptr;
        
        if(top >0 && duk_is_string(ctx, -top)) {
            url = duk_to_string(ctx, -top);
        }
        
        if(top >1 && duk_is_string(ctx, -top + 1)) {
            protocol = duk_to_string(ctx, -top + 1);
        }
        
        if(url) {
            
            kk::Strong v = (kk::Object *) (new WebSocket());
            
            WebSocket * vv = v.as<WebSocket>();
            
            vv->open(ev_base(ctx),ev_dns(ctx),url,protocol);
            
            kk::script::PushObject(ctx, (kk::Object *) vv);
            
            return 1;
        }
        
        return 0;
    }
    
    IMP_SCRIPT_CLASS_BEGIN(nullptr, WebSocket, WebSocket)
    
    static kk::script::Method methods[] = {
        {"on",(kk::script::Function) &WebSocket::duk_on},
        {"close",(kk::script::Function) &WebSocket::duk_close},
        {"send",(kk::script::Function) &WebSocket::duk_send},
    };
    
    kk::script::SetMethod(ctx, -1, methods, sizeof(methods) / sizeof(kk::script::Method));
    
    duk_push_string(ctx, "alloc");
    duk_push_c_function(ctx, WebSocketAlloc, 2);
    duk_put_prop(ctx, -3);
   
    IMP_SCRIPT_CLASS_END
    
    WebSocket::WebSocket()
        :_bev(nullptr),_bodyType(WebSocketTypeNone)
            ,_state(WebSocketStateNone),_bodyLength(0) {
        _body = evbuffer_new();
    }
    
    WebSocket::~WebSocket() {
        if(_bev != nullptr) {
            bufferevent_free(_bev);
            _bev = nullptr;
        }
        evbuffer_free(_body);
    }
    
    void WebSocket::close() {
        if(_bev != nullptr) {
            bufferevent_free(_bev);
            _bev = nullptr;
        }
    }

    void WebSocket_data_rd(struct bufferevent *bev, void *ctx) {
        WebSocket * v = (WebSocket *) ctx;
        v->onReading();
    }
    
    void WebSocket_data_wd(struct bufferevent *bev, void *ctx) {
        WebSocket * v = (WebSocket *) ctx;
        v->onWritting();
    }
    
    void WebSocket_event_cb(struct bufferevent *bev, short what, void *ctx) {
        WebSocket * v = (WebSocket *) ctx;
        if(what & BEV_EVENT_CONNECTED) {
            v->onConnected();
        } else if(what & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)){
            v->onClose(nullptr);
        }
    }
    
    void WebSocket_evdns_cb (int result, char type, int count, int ttl, void *addresses, void *arg) {
        
        WebSocket * v = (WebSocket *) arg;
        
        struct in_addr * addr = (struct in_addr *) addresses;
        
        if(result == DNS_ERR_NONE && count > 0) {
            v->onResolve(addr);
        } else {
            v->onClose("域名解析错误");
        }
    }

    void WebSocket::onResolve(struct in_addr * addr){
        if(_state == WebSocketStateNone) {
            _state = WebSocketStateResolve;
            _addr.sin_addr = * addr;
            bufferevent_socket_connect(_bev, (struct sockaddr *) &_addr, sizeof(struct sockaddr_in));
        }
    }
    
    void WebSocket::onConnected() {
        if(_state == WebSocketStateResolve) {
            _state = WebSocketStateConnected;
            bufferevent_enable(_bev, EV_WRITE);
        }
    }
    
    void WebSocket::onWritting() {
        if(_state == WebSocketStateConnected) {
            bufferevent_enable(_bev, EV_READ);
        }
    }
    
    void WebSocket::onData(WebSocketType type,void * data,size_t length) {
        if(type == WebSocketTypePing) {
        
            
        } else {
            {
                
                std::map<duk_context *,void *>::iterator i = _heapptrs.begin();
                
                while(i != _heapptrs.end()) {
                    
                    duk_context * ctx = i->first;
                    
                    duk_push_heapptr(ctx, i->second);
                    
                    duk_push_sprintf(ctx, "_ondata");
                    duk_get_prop(ctx, -2);
                    
                    if(duk_is_function(ctx, -1)) {
                        
                        if(type == WebSocketTypeText) {
                            
                            duk_push_lstring(ctx, (const char *) data,length);
                            
                            if(duk_pcall(ctx, 1) != DUK_EXEC_SUCCESS) {
                                kk::script::Error(ctx, -1);
                            }
                            
                        } else {
                            
                            void * d = duk_push_fixed_buffer(ctx, length);
                            
                            memcpy(d, data, length);
                            
                            duk_push_buffer_object(ctx, -1, 0, length, DUK_BUFOBJ_UINT8ARRAY);
                            
                            duk_remove(ctx, -2);
                            
                            if(DUK_EXEC_SUCCESS != duk_pcall(ctx, 1)) {
                                kk::script::Error(ctx, -1);
                            }
                            
                        }

                    }
                    
                    duk_pop_n(ctx, 2);
                    
                    i ++;
                }
                
            }
        }
    }
    
    void WebSocket::onReading() {
        
        if(_state == WebSocketStateConnected) {
            
            evbuffer * data = bufferevent_get_input(_bev);
            
            char * s = (char *) EVBUFFER_DATA(data);
            char * e = s + EVBUFFER_LENGTH(data);
            char * p = s;
            int n = 0;
            
            while(p != e) {
                if(*p == '\r') {
                    
                } else if(*p == '\n') {
                    n++;
                    if(n == 2) {
                        p ++;
                        break;
                    }
                } else {
                    n = 0;
                }
                p ++ ;
            }
            
            if(n == 2) {
                
                int code = 0;
                char status[128];
                
                sscanf(s, "HTTP/1.1 %d %[^\r\n]\r\n",&code,status);
                
                evbuffer_drain(data, p - s);
                
                if(code == 101) {
                    onOpen();
                    return;
                } else {
                    onClose(status);
                    return ;
                }

            }
            
            bufferevent_enable(_bev, EV_READ);
            
        } else if(_state == WebSocketStateOpened) {
            
            evbuffer * data = bufferevent_get_input(_bev);
            uint8_t * p = EVBUFFER_DATA(data);
            size_t n = EVBUFFER_LENGTH(data);
            
            if(_bodyType != WebSocketTypeNone && _bodyLength > 0 && n > 0) {
                
                ssize_t v = (ssize_t) _bodyLength - (ssize_t) EVBUFFER_LENGTH(_body);
                
                if(v > 0 && n >= v) {
                    evbuffer_add(_body, p, v);
                    evbuffer_drain(data, v);
                    p = EVBUFFER_DATA(data);
                    n = EVBUFFER_LENGTH(data);
                }
                
                if(EVBUFFER_LENGTH(_body) == _bodyLength){
                    onData(_bodyType, EVBUFFER_DATA(_body), EVBUFFER_LENGTH(_body));
                    evbuffer_drain(_body, EVBUFFER_LENGTH(_body));
                    _bodyType = WebSocketTypeNone;
                    _bodyLength = 0;
                } else {
                    bufferevent_enable(_bev, EV_READ);
                    return;
                }
            }
            
            if(n >= 2) {
                
                bool isFin = (KKFinMask & p[0]);
                uint8_t receivedOpcode = KKOpCodeMask & p[0];
                bool isMasked = (KKMaskMask & p[1]);
                uint8_t payloadLen = (KKPayloadLenMask & p[1]);
                int offset = 2;
                
                if((isMasked  || (KKRSVMask & p[0])) && receivedOpcode != WebSocketOpCodePong) {
                    this->onClose("不支持的协议");
                    this->close();
                    return;
                }
                
                bool isControlFrame = (receivedOpcode == WebSocketOpCodeConnectionClose || receivedOpcode == WebSocketOpCodePing);
                
                if(!isControlFrame && (receivedOpcode != WebSocketOpCodeBinaryFrame && receivedOpcode != WebSocketOpCodeContinueFrame && receivedOpcode != WebSocketOpCodeTextFrame && receivedOpcode != WebSocketOpCodePong)) {
                    this->onClose("不支持的协议");
                    this->close();
                    return;
                }
                
                if(isControlFrame && !isFin) {
                    this->onClose("不支持的协议");
                    this->close();
                    return;
                }
                
                if(receivedOpcode == WebSocketOpCodeConnectionClose) {
                    this->onClose(nullptr);
                    this->close();
                    return;
                }
                
                if(isControlFrame && payloadLen > 125) {
                    this->onClose("不支持的协议");
                    this->close();
                    return;
                }
                
                uint64_t dataLength = payloadLen;
                if(payloadLen == 127) {
                    dataLength =  ntohll((*(uint64_t *)(p+offset)));
                    offset += sizeof(uint64_t);
                } else if(payloadLen == 126) {
                    dataLength = ntohs(*(uint16_t *)(p+offset));
                    offset += sizeof(uint16_t);
                }
                
                if(n < offset) { // we cannot process this yet, nead more header data
                    bufferevent_enable(_bev, EV_READ);
                    return;
                }
                
                uint64_t len = dataLength;
                
                if(dataLength > (n-offset) || (n - offset) < dataLength) {
                    len = n-offset;
                }

                if(receivedOpcode == WebSocketOpCodePong) {
                    evbuffer_drain(data, (size_t) (offset + len));
                    onReading();
                    return;
                }
                
                if(receivedOpcode == WebSocketOpCodeContinueFrame) {
                    evbuffer_add(_body, p + offset, (size_t) len);
                    evbuffer_drain(data, (size_t) (offset + len));
                } else if(receivedOpcode == WebSocketOpCodeTextFrame) {
                    _bodyType = WebSocketTypeText;
                    evbuffer_add(_body, p + offset, (size_t) len);
                    evbuffer_drain(data, (size_t) (offset + len));
                } else if(receivedOpcode == WebSocketOpCodeBinaryFrame) {
                    _bodyType = WebSocketTypeBinary;
                    evbuffer_add(_body, p + offset, (size_t) len );
                    evbuffer_drain(data, (size_t) (offset + len) );
                } else if(receivedOpcode == WebSocketOpCodePing) {
                    _bodyType = WebSocketTypePing;
                    evbuffer_add(_body, p + offset, (size_t) len);
                    evbuffer_drain(data, (size_t) (offset + len) );
                } else {
                    this->onClose("不支持的协议");
                    this->close();
                    return;
                }
                
                if(isFin && EVBUFFER_LENGTH(_body) == dataLength) {
                    onData(_bodyType, EVBUFFER_DATA(_body), EVBUFFER_LENGTH(_body));
                    evbuffer_drain(_body, EVBUFFER_LENGTH(_body));
                    _bodyType = WebSocketTypeNone;
                    _bodyLength = 0;
                } else {
                    _bodyLength = dataLength;
                }
                
                onReading();
                
                return;
            }
            
            bufferevent_enable(_bev, EV_READ);
        }
        
    }
    
    void WebSocket::onOpen() {
        if(_state == WebSocketStateConnected) {
            _state = WebSocketStateOpened;
            
            {
                
                std::map<duk_context *,void *>::iterator i = _heapptrs.begin();
                
                while(i != _heapptrs.end()) {
                    
                    duk_context * ctx = i->first;
                    
                    duk_push_heapptr(ctx, i->second);
                    
                    duk_push_sprintf(ctx, "_onopen");
                    duk_get_prop(ctx, -2);
                    
                    if(duk_is_function(ctx, -1)) {
                        
                        if(duk_pcall(ctx, 0) != DUK_EXEC_SUCCESS) {
                            kk::script::Error(ctx, -1);
                        }
                        
                    }
                    
                    duk_pop_n(ctx, 2);
                    
                    i ++;
                }
                
            }
            
            onReading();
            if(EVBUFFER_LENGTH(bufferevent_get_output(_bev)) > 0) {
                bufferevent_enable(_bev, EV_WRITE);
            }
        }
    }
    
    void WebSocket::onClose(kk::CString errmsg) {
        if(_state != WebSocketStateClosed) {
            _state = WebSocketStateClosed;

            retain();
            
            {
                
                std::map<duk_context *,void *> m;
                
                std::map<duk_context *,void *>::iterator i = _heapptrs.begin();
                
                while(i != _heapptrs.end()) {
                    
                    duk_context * ctx = i->first;
        
                    duk_push_global_object(ctx);
                    duk_push_sprintf(ctx, "0x%x",(long) i->second);
                    duk_push_heapptr(ctx, i->second);
                    duk_put_prop(ctx, -3);
                    duk_pop(ctx);
                    
                    m[i->first] = i->second;
                    
                    i ++;
                }
                
                i = m.begin();
                
                while(i != m.end()) {
                    
                    duk_context * ctx = i->first;
                    
                    duk_push_heapptr(ctx, i->second);
                    
                    duk_push_sprintf(ctx, "_onclose");
                    duk_get_prop(ctx, -2);
                    
                    if(duk_is_function(ctx, -1)) {
                        
                        duk_push_string(ctx, errmsg);
                        
                        if(duk_pcall(ctx, 1) != DUK_EXEC_SUCCESS) {
                            kk::script::Error(ctx, -1);
                        }
                        
                    }
                    
                    duk_pop_n(ctx, 2);
                    
                    duk_push_global_object(ctx);
                    duk_push_sprintf(ctx, "0x%x",(long) i->second);
                    duk_del_prop(ctx, -2);
                    duk_pop(ctx);
                    
                    i ++;
                }
                
            }
            
            if(_bev != nullptr) {
                bufferevent_free(_bev);
                _bev = nullptr;
            }
            
            release();
        }
    }
    
 
    void WebSocket::send(void * data,size_t n) {
        send(data,n,WebSocketTypeBinary);
    }
    
    void WebSocket::send(kk::CString text) {
        send((void *) text, (size_t) strlen(text),WebSocketTypeText);
    }
    
    void WebSocket::send(void * data,size_t n,WebSocketType type) {
        
        if(_bev == nullptr) {
            return;
        }
        
        uint8_t frame[KKMaxFrameSize];
        
        memset(frame, 0, sizeof(frame));
        
        switch (type) {
            case WebSocketTypePing:
                frame[0] = KKFinMask | WebSocketOpCodePing;
                break;
            case WebSocketTypeText:
                frame[0] = KKFinMask | WebSocketOpCodeTextFrame;
                break;
            case WebSocketTypeBinary:
                frame[0] = KKFinMask | WebSocketOpCodeBinaryFrame;
                break;
            default:
                return;
        }
        
        uint64_t offset = 2;
        
        if(n < 126) {
            frame[1] |= n;
        } else if(n <= UINT16_MAX) {
            frame[1] |= 126;
            *((uint16_t *)(frame + offset)) = htons((uint16_t)n);
            offset += sizeof(uint16_t);
        } else {
            frame[1] |= 127;
            *((uint64_t *)(frame + offset)) = htonll((uint64_t)n);
            offset += sizeof(uint64_t);
        }
        
        frame[1] |= KKMaskMask;
        uint8_t *mask_key = (frame + offset);
        for(int i=0;i<sizeof(uint32_t);i++) {
            mask_key[i] = rand();
        }
        offset += sizeof(uint32_t);
        
        evbuffer * output = bufferevent_get_output(_bev);
        evbuffer_add(output, frame, (size_t) offset);
        
        uint8_t * p = (uint8_t *) data;
        uint8_t u;
        
        for(int i=0;i<n;i++) {
            u = p[i] ^ mask_key[i % sizeof(uint32_t)];
            evbuffer_add(output, &u, 1);
        }
        
        bufferevent_enable(_bev, EV_WRITE);
    }
    
    kk::Boolean WebSocket::open(event_base * base,evdns_base * dns, kk::CString url,kk::CString protocol) {
        
        evhttp_uri * uri = evhttp_uri_parse(url);
        
        if(uri == nullptr) {
            _errmsg = "错误的URL";
            return false;
        }
        
        kk::String host = evhttp_uri_get_host(uri);
        kk::String origin = evhttp_uri_get_scheme(uri);
        kk::String path = evhttp_uri_get_path(uri);
        
        if(path == "") {
            path = "/";
        }
        
        kk::String query = evhttp_uri_get_query(uri);
        
        if(query != "") {
            path.append("?");
            path.append(query);
        }
        
        if(origin == "ws") {
            origin = "http";
        }
        
        if(origin == "wss") {
            origin = "https";
        }
        
        origin.append("://");
        origin.append(host);
        
        char fmt[64];
        
        int port = evhttp_uri_get_port(uri);
        
        if(port == 0) {
            port = 80;
        } else {
            host.append(":");
            snprintf(fmt, sizeof(fmt), "%d",port);
            host.append(fmt);
            
            origin.append(":");
            origin.append(fmt);
        }
        
        memset(&_addr, 0, sizeof(_addr));
        
        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(port);
        
        _bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
        
        bufferevent_setcb(_bev, WebSocket_data_rd, WebSocket_data_wd, WebSocket_event_cb, this);
        
        bufferevent_setwatermark(_bev, EV_READ | EV_WRITE, 0, MAX_BUF_SIZE);
        
        evbuffer * data = bufferevent_get_output(_bev);
        
        evbuffer_add_printf(data, "GET %s HTTP/1.1\r\n", path.c_str());
        evbuffer_add_printf(data, "Host: %s\r\n",host.c_str());
        evbuffer_add_printf(data, "Upgrade: %s\r\n","websocket");
        evbuffer_add_printf(data, "Connection: %s\r\n","Upgrade");
        evbuffer_add_printf(data, "Origin: %s\r\n",origin.c_str());
        evbuffer_add_printf(data, "Sec-WebSocket-Key: %s\r\n",Sec_WebSocket_Key);
        evbuffer_add_printf(data, "Sec-WebSocket-Version: %s\r\n","13");
        evbuffer_add_printf(data, "\r\n");
    
        struct in_addr addr;
        
        addr.s_addr = inet_addr(evhttp_uri_get_host(uri));
        
        if(addr.s_addr == INADDR_NONE) {
            evdns_base_resolve_ipv4(dns, evhttp_uri_get_host(uri), 0, WebSocket_evdns_cb, this);
        } else {
            onResolve(&addr);
        }
        
        evhttp_uri_free(uri);
        
        return true;
    }
    
    duk_ret_t WebSocket::duk_on(duk_context * ctx) {
        
        void *heapptr = this->heapptr(ctx);
        
        if(heapptr) {
            
            int top = duk_get_top(ctx);
            
            if(top > 0 && duk_is_string(ctx, -top) ) {
                
                const char *name = duk_to_string(ctx, -top);
                
                if(top > 1 && duk_is_function(ctx, -top + 1)) {
                    duk_push_heapptr(ctx, heapptr);
                    duk_push_sprintf(ctx, "_on%s",name);
                    duk_dup(ctx, - top + 1 - 2);
                    duk_put_prop(ctx, -3);
                    duk_pop(ctx);
                } else {
                    duk_push_heapptr(ctx, heapptr);
                    duk_push_sprintf(ctx, "_on%s",name);
                    duk_del_prop(ctx, -2);
                    duk_pop(ctx);
                }
                
            }
        }
        
        return 0;
    }
    
    duk_ret_t WebSocket::duk_close(duk_context * ctx) {
        
        close();
        
        return 0;
    }
    
    duk_ret_t WebSocket::duk_send(duk_context * ctx) {
        
        int top = duk_get_top(ctx);
        
        if(top >0 ) {
            if(duk_is_string(ctx, - top)) {
                send(duk_to_string(ctx, -top));
            } else if(duk_is_buffer_data(ctx, -top)) {
                duk_size_t n;
                void * bytes = duk_get_buffer_data(ctx, - top, &n);
                send(bytes, n);
            }
        }
        
        return 0;
        
    }
    
}

