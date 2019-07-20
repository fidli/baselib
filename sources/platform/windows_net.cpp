#ifndef WINDOWS_NET
#define WINDOWS_NET

//include these at the top of the file, even before windows.h
/*
#include <winsock2.h>
#include <ws2tcpip.h>
*/
#include "common.h"
#include "util_net.h"


WSADATA socketsContext;

struct NetSocket : NetSocketSettings{
    SOCKET socket;
    bool valid;
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


bool isSocketHandleValid(NetSocket * target){
    return target->valid;
}

bool closeSocket(NetSocket * target){
    int shutdownRes = shutdown(target->socket, SD_BOTH);
    int closesocketRes = closesocket(target->socket);
    return closesocketRes == 0  && shutdownRes  == 0;
}

bool initSocket(NetSocket * target, const char * ipAddress, const char * port, const NetSocketSettings * settings){
    target->valid = false;
    target->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    int err = WSAGetLastError();
    
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
            bool found = false;
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
        target->valid = true;
        *((NetSocketSettings *) &target) = *settings;
        return true;
    }
    else{
        return false;
    }
}


bool openSocket(NetSocket * target, const NetSocketSettings * settings){
    target->valid = false;
    target->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    int err = WSAGetLastError();
    
    if(settings->blocking == false){
        if(!setSocketOption(target->socket, FIONBIO, 1)){
            closeSocket(target);
            return false;
        }
    }
    
    
    if(target->socket != INVALID_SOCKET){
        *((NetSocketSettings *) &target) = *settings;
        return true;
    }else{
        return false;
    }
}




bool tcpListen(const NetSocket * server, uint16 maxConnections){
    return listen(server->socket, maxConnections) == 0;
}

bool tcpAccept(const NetSocket * server, NetSocket * client, const NetSocketSettings * clientSettings){
    //sockaddr * clientInfo = NULL;
    client->socket = accept(server->socket, NULL, NULL);
    if (client->socket != INVALID_SOCKET){
        if(client->socket == WSAEWOULDBLOCK){
            client->valid = false;
            return true;
        }
        if(clientSettings->blocking == false){
            if(!setSocketOption(client->socket, FIONBIO, 1)){
                closeSocket(client);
                return false;
            }
            
        }else{
            if(!setSocketOption(client->socket, FIONBIO, 0)){
                closeSocket(client);
                return false;
            }
        }
        client->valid = true;
        *((NetSocketSettings *) &client) = *clientSettings;
        return true;
    }else{
        return false;
    }
}

bool tcpConnect(NetSocket * source, const char * ip, const char * port){
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    addrinfo * result;
    if(getaddrinfo(ip, port, &hints, &result) == 0){
        bool found = false;
        addrinfo * actual = result;
        while(actual != NULL){
            if(hints.ai_family == actual->ai_family && hints.ai_socktype ==  actual->ai_socktype &&hints.ai_protocol == actual->ai_protocol){
                found = true;
                break;
            }
            actual = actual->ai_next;
        }
        
        if(found){
            int resultInt = connect(source->socket, actual->ai_addr, actual->ai_addrlen);
            bool ret = false;
            if(!source->blocking){
                int error = WSAGetLastError();
                if(error == WSAEWOULDBLOCK){
                    fd_set tmpSet;
                    FD_ZERO(&tmpSet);
                    FD_SET(source->socket, &tmpSet);
                    int selectRes = select(0, &tmpSet, &tmpSet, NULL, NULL);
                    if(selectRes == 1){
                        ret = true;
                    }else if(selectRes == 0){
                        //NOTE(AK): this means that the timeout has expired, but it was supposed to be inifinite
                        INV;
                    }else{
                        //WSAGetLastError(); for debugging
                        //https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-select
                        ret = false;
                    }
                }else{
                    ret = false;
                }
            }else{
                ret = resultInt == 0;
            }
            freeaddrinfo(result);
            return ret;
        }
    }
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