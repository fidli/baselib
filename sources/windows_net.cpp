#ifndef WINDOWS_NET
#define WINDOWS_NET

#include "util_net.h"

WSADATA socketsContext;

struct NetSocket{
    SOCKET socket;
};


static bool setSocketOption(SOCKET socket, long cmd, u_long arg){
    return ioctlsocket(socket, cmd, &arg) == 0;
}

bool initNet(){
    return WSAStartup(MAKEWORD(2, 2), &socketsContext) == 0;
}

bool closeNet(){
    WSACleanup();
    return true;
}


bool closeSocket(NetSocket * target){
    int shutdownRes = shutdown(target->socket, SD_BOTH);
    int closesocketRes = closesocket(target->socket);
    return closesocketRes == 0  && shutdownRes  == 0;
}

bool initSocket(NetSocket * target, const char * ipAddress, const char * port, const NetSocketSettings * settings){
    
    target->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(settings->blocking == false){
        if(!setSocketOption(target->socket, FIONBIO, 1)){
            closeSocket(target);
            return false;
        }
    }
    
    
    if(target->socket != INVALID_SOCKET){
        
        addrinfo hints = {}; 
        
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        
        addrinfo * result = NULL;
        if(getaddrinfo(ipAddress, port, &hints, &result) == 0){
            
            addrinfo * actual = result;
            while(actual != NULL){
                if(hints.ai_family == actual->ai_family && hints.ai_socktype ==  actual->ai_socktype &&hints.ai_protocol == actual->ai_protocol){
                    found = true;
                    break;
                }
                actual = actual->ai_next;
            }
            
            if(!found){
                closeSocket(target);
                freeaddrinfo(result);
                return false;
            }
            
            if(bind(target->socket, actual->ai_addr, actual->ai_addrlen) == 0){
                freeaddrinfo(result);
                return true;
            }else{
                closeSocket(target);
                freeaddrinfo(result);
                return false;
            }
            
            
        }
        else{
            closeSocket(target);
            return false;
        }
        return true;
    }
    else{
        return false;
    }
}



bool tcpListen(const NetSocket * server, uint16 maxConnections){
    return listen(server->socket, maxConnections) == 0;
}

bool tcpAccept(const NetSocket * server, NetSocket * client, const NetSocketSettings * clientSettings){
    sockaddr * clientInfo;
    client->socket = accept(server->socket, clientInfo, NULL);
    if (client->socket != INVALID_SOCKET){
        if(clientSettings->blocking == false){
            if(!setSocketOption(client->socket, FIONBIO, 1)){
                closeSocket(client);
                return false;
            }
        }
        return true;
    }else{
        return false;
    }
}

bool tcpConnect(const NetSocket * source, const char * ip, const char * port){
    ASSERT(false);
    return false;
}

NetResultType netRecv(const NetSocket * target, NetRecvResult * result){
    result->resultLength = recv(target->socket, result->buffer, result->bufferLength, 0);
    if(result->resultLength == 0){
        return NetResultType_Closed;
    }else if(result->resultLength == INVALID_SOCKET){
        int error = WSAGetLastError();
        if(error  == WSAECONNABORTED){
            return NetResultType_Timeout;
        }else if(error == WSAEWOULDBLOCK){
            //non blocking, no data
            result->resultLength = 0;
            return NetResultType_Ok;
        }else{
            return NetResultType_Error;
        }
        
    }
    return NetResultType_Ok;
}

NetResultType netSend(const NetSocket * target, const NetSendSource * source){
    int result = send(target->socket, (const char *) source->buffer, source->bufferLength, 0);
    if(result != source->bufferLength){
        if(result == SOCKET_ERROR){
            int error = WSAGetLastError();
            if(error  == WSAECONNABORTED){
                return NetResultType_Timeout;
            }else{
                return NetResultType_Error;
            }
        }else{
            return NetResultType_Error;
        }
    }
    
    return NetResultType_Ok;
}

#endif