#ifndef UTIL_NET
#define UTIL_NET

struct NetSocket;

enum NetResultType{
    NetResultType_Error,
    NetResultType_Ok,
    NetResultType_Timeout,
    NetResultType_Closed
};

struct NetRecvResult{
    char * buffer;
    int32 bufferLength;
    int32 resultLength;
};

struct NetSendSource{
    char * buffer;
    int32 bufferLength;
};

struct NetSocketSettings{
    bool blocking;
    bool reuseAddr;
};

void invalidateSocketHandle(NetSocket * target);
bool isSocketHandleValid(NetSocket * target);

bool initNet();
bool closeNet();

bool openSocket(NetSocket * target, const NetSocketSettings * settings);
bool initSocket(NetSocket * target, const char * ipAddress, const char * port, const NetSocketSettings * settings);
bool closeSocket(NetSocket * target);

bool tcpListen(const NetSocket * server, uint16 maxConnections);
bool tcpAccept(const NetSocket * server, NetSocket * client, const NetSocketSettings * clientSettings);
bool tcpConnect(const NetSocket * source, const char * ip, const char * port);

NetResultType netRecv(const NetSocket * target, NetRecvResult * result);
NetResultType netSend(const NetSocket * target, const NetSendSource * source);


#endif