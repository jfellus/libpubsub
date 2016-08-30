/*
 * websocket.cpp
 *
 *  Created on: 14 juil. 2016
 *      Author: jfellus
 */


#include "websocket.h"

static IWebSocketServer* curWebSocketServer = NULL;



static int callback_http(struct libwebsocket_context* _this, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len) {return 0;}
static int callback_websocket(struct libwebsocket_context *context, struct libwebsocket *ws, enum libwebsocket_callback_reasons reason, void *user, void *msg, size_t len);


static struct libwebsocket_protocols protocols[] = {
		{"http-only",callback_http,0},
		{"pub-sub", callback_websocket, sizeof(IWebSocketPeer*), 0},
		{ NULL, NULL, 0, 0 } /* terminator */
};


IWebSocketServer::IWebSocketServer(int port) {
	curWebSocketServer = this;
	lws_set_log_level(0, NULL);

	// setup websocket context info
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);
	info.port = port;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	context = libwebsocket_create_context(&info);
	if (context == NULL) throw "Websocket context create error";
}

void IWebSocketServer::run() {
	while ( !bQuit ) libwebsocket_service(context, 50);
	usleep(10);
	libwebsocket_context_destroy(context);
}

void IWebSocketServer::start() {
	thread = std::thread([&]() { run(); });
}

static int callback_websocket(struct libwebsocket_context *context, struct libwebsocket *ws, enum libwebsocket_callback_reasons reason, void *user, void *msg, size_t len) {
	IWebSocketPeer* peer = user ? *(IWebSocketPeer**)user : NULL;
	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
		*(IWebSocketPeer**)user = curWebSocketServer->createPeer(ws);
		break;
	case LWS_CALLBACK_RECEIVE:
		peer->onMessage((char*)msg);
		break;
	case LWS_CALLBACK_CLOSED:
		delete peer;
		break;
	}
	return 0;
}


////////////////////
// IWebSocketPeer //
////////////////////

int IWebSocketPeer::send(const char* msg) {
	int len = strlen(msg);
	unsigned char* _out_buf = new unsigned char [LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING];
	memcpy (_out_buf + LWS_SEND_BUFFER_PRE_PADDING, msg, len );

	libwebsocket_write(ws, _out_buf + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);
	delete _out_buf;
	return 0;
}
