//
//  kk-ws.h
//  app
//
//  Created by hailong11 on 2018/6/8.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#ifndef kk_ws_h
#define kk_ws_h

#if defined(__APPLE__)

#include <KKSurface/kk-ev.h>
#include <KKSurface/kk-script.h>

#elif defined(__ANDROID_API__)

#include "kk-script.h"
#include "kk-ev.h"

#endif

#ifdef  __cplusplus
extern "C" {
#endif
    
#include <sys/queue.h>
#include <arpa/inet.h>

#ifdef  __cplusplus
}
#endif



namespace kk {
    
    enum WebSocketState {
        WebSocketStateNone,WebSocketStateResolve,WebSocketStateConnected,WebSocketStateOpened,WebSocketStateClosed
    };
    
    enum WebSocketType {
        WebSocketTypeNone,WebSocketTypeText,WebSocketTypeBinary,WebSocketTypePing
    };
    
    class WebSocket : public kk::script::HeapObject, public kk::script::IObject {
    public:
        WebSocket();
        virtual ~WebSocket();
        
        virtual kk::Boolean open(event_base * base,evdns_base * dns, kk::CString url,kk::CString protocol);
        virtual void close();
        virtual void send(void * data,size_t n,WebSocketType type);
        virtual void send(void * data,size_t n);
        virtual void send(kk::CString text);
        
        virtual duk_ret_t duk_on(duk_context * ctx);
        virtual duk_ret_t duk_close(duk_context * ctx);
        virtual duk_ret_t duk_send(duk_context * ctx);
        
        DEF_SCRIPT_CLASS
        
    protected:
        kk::String _errmsg;
        bufferevent * _bev;
        WebSocketState _state;
        evbuffer * _body;
        WebSocketType _bodyType;
        int64_t _bodyLength;
        struct sockaddr_in _addr;
        
        virtual void onClose(kk::CString errmsg);
        virtual void onResolve(struct in_addr * addr);
        virtual void onConnected();
        virtual void onWritting();
        virtual void onReading();
        virtual void onOpen();
        virtual void onData(WebSocketType type,void * data,size_t length);
        
        friend void WebSocket_data_rd(struct bufferevent *bev, void *ctx);
        friend void WebSocket_data_wd(struct bufferevent *bev, void *ctx);
        friend void WebSocket_event_cb(struct bufferevent *bev, short what, void *ctx);
        friend void WebSocket_evdns_cb (int result, char type, int count, int ttl, void *addresses, void *arg);
    };
    
}

#endif /* kk_ws_h */
