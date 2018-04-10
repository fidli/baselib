#ifndef LINUX_NET
#define LINUX_NET

#include "util_net.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <poll.h>

struct NetSocket{
    int socket;
};


bool initNet(){
    return true;
}

bool closeNet(){
    return true;
}


static bool setSocketOptionSO(int socket, int cmd, int arg){
    return setsockopt(socket, SOL_SOCKET, cmd, &arg, sizeof(arg)) >= 0;
}

static bool setSocketOption(int socket, long cmd, u_long arg){
    return ioctl(socket, cmd, (char *)&arg) >= 0;
}

void invalidateSocketHandle(NetSocket * target){
    target->socket = -1;
}

bool isSocketHandleValid(NetSocket * target){
    return target->socket != -1;
}

bool closeSocket(NetSocket * target){
    int res = shutdown(target->socket, SHUT_RDWR);
    if(res == -1 && (errno == ENOTCONN || errno == EBADF)){
        res = 0;
    }
    res |= close(target->socket);
    if(res == -1 && (errno == ENOTCONN || errno == EBADF)){
        res = 0;
    }
    invalidateSocketHandle(target);
    return res == 0;
}

bool setSocketSettings(NetSocket * target, const NetSocketSettings * settings){
    if(settings->blocking == false){
        if(!setSocketOption(target->socket, FIONBIO, 1)){
            
            return false;
        }
    }
    if(settings->reuseAddr == true){
        if(!setSocketOptionSO(target->socket, SO_REUSEADDR, 1)){
            
            return false;
        }
        /*if(!setSocketOptionSO(target->socket, SO_REUSEPORT, 1)){
        
            return false;
        }*/
        
    }
    return true;
}

bool openSocket(NetSocket * target, const NetSocketSettings * settings){
    
    target->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(target->socket != -1){
        if(!setSocketSettings(target, settings)){
            closeSocket(target);
            return false;
        }
        return true;
    }
    
    return false;
}

bool initSocket(NetSocket * target, const char * ipAddress, const char * port, const NetSocketSettings * settings){
    
    if(openSocket(target, settings)){
        
        
        
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
    int result = accept(server->socket, NULL, NULL);
    if (result != -1){
        client->socket = result;
        if(!setSocketSettings(client, clientSettings)){
            closeSocket(client);
            return false;
        }
        return true;
    }else{
        return false;
    }
}

bool tcpConnect(const NetSocket * source, const char * ip, const char * port){
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
            bool ret = connect(source->socket, actual->ai_addr, actual->ai_addrlen) == 0;
            if(!ret && errno == EINPROGRESS){
                //this branch is fucked... either the target exists, is non blocking and connection is estabilishing, OR target does not exist, timeout of 1s should resolve this
                //MAN says:
                /*
                
                EINPROGRESS
The socket is nonblocking and the connection cannot be completed immediately. It is possible to select(2) or poll(2) for completion by selecting the socket for writing. After select(2) indicates writability, use getsockopt(2) to read the SO_ERROR option at level SOL_SOCKET to determine whether connect() completed successfully (SO_ERROR is zero) or unsuccessfully (SO_ERROR is one of the usual error codes listed here, explaining the reason for the failure).

*/
                
                pollfd arg;
                arg.fd = source->socket;
                arg.events = POLLOUT | POLLIN;
                int pollres = poll(&arg, 1, 2000);
                if(pollres > 0){
                    if((arg.revents & POLLPRI) ||
                       (arg.revents & POLLRDHUP) ||
                       (arg.revents & POLLERR) ||
                       (arg.revents & POLLHUP) ||
                       (arg.revents & POLLNVAL)){
                        ret = false;
                        
                    }else{
                        ret = true;
                    }
                }else{
                    ret = false;
                }
                
            }
            freeaddrinfo(result);
            return ret;
        }
    }
    return false;
}

NetResultType netRecv(const NetSocket * target, NetRecvResult * result){
    result->resultLength = recv(target->socket, result->buffer, result->bufferLength, MSG_NOSIGNAL);
    if(result->resultLength == -1){
        int error = errno;
        
        if(error == EWOULDBLOCK){
            //non blocking, no data
            result->resultLength = 0;
            return NetResultType_Ok;
        }else if(error == EPIPE){
            //epipe = broken pipe - hard cancellation?
            //cancelled
            return NetResultType_Closed;
        }
        else{
            return NetResultType_Error;
        }
        
    }
    
    return NetResultType_Ok;
}

NetResultType netSend(const NetSocket * target, const NetSendSource * source){
    
    int result = send(target->socket, (const char *) source->buffer, source->bufferLength, MSG_NOSIGNAL);
    if(result != source->bufferLength){
        if(result == -1){
            int error = errno;
            if(error  == EAGAIN){
                return NetResultType_Timeout;
            }else if(error == EPIPE){
                //epipe = broken pipe - hard cancellation?
                //cancelled
                return NetResultType_Closed;
            }
            else{
                return NetResultType_Error;
            }
        }else{
            return NetResultType_Error;
        }
    }
    
    return NetResultType_Ok;
}

#endif