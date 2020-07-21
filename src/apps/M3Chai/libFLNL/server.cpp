/**
 * \file server.cpp
 * \brief Network server class implementation
 * \author Vincent Crocher
 * \version 0.8
 * \date July 2020
 *
 *
 */

#include "FLNL.h"


/*-----------------------------------------------------------------------------------------------------------------*/
/*###################################   CONNECTING AND DISCONNECTING METHODS    ###################################*/
/*-----------------------------------------------------------------------------------------------------------------*/
//! Open a connection and wait for a client launching an Accepting thread
//! \param addr : The server ip address (local address)
//! \return 0 if OK
//! \return -1 if bind() error
//! \return -2 if error creating the accepting thread
int server::Connect(char * addr)
{
    //Initialise server address
    sin.sin_addr.s_addr = inet_addr(addr);

    //Initialise server socket
    ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

    //Initialise
    Waiting=false;
    if(bind(ServerSocket, (struct sockaddr*)&sin, sizeof(sin))==-1) {
        #ifdef WINDOWS
            printf("WSA Error code : %d\t", WSAGetLastError());
        #endif
        perror("FLNL::server::bind() :");
        close(ServerSocket);
        return -1;
    }
    else {
       return Reconnect();
    }
}

//! Wait for a client launching an Accepting thread. Can be called if socket initialised but disconnected
//! \return 0 if OK
//! \return -1 if already connected or already waiting
//! \return -2 if error creating the accepting thread
int server::Reconnect()
{
    if(IsConnected() || Waiting) {
        return -1;
    }

    //Ecoute d'une seule connexion
    listen(ServerSocket, 1);

    //Creation d'un thread d'attente de connexion du client
    Waiting=true;
    if(pthread_create(&AcceptingThread, NULL, accepting, (void*)this)!=0) {
        Waiting=false;
        printf("FLNL::server::Connect() : Error creating accepting thread\n");
        return -2;
    }

    //OK
    return 0;
}

//! Disconnect and close the socket
//! \return the close(Socket) return value
int server::Disconnect()
{
    Connected=false;
    Waiting=false;
    pthread_join(ReceivingThread, NULL);
    pthread_join(AcceptingThread, NULL);

    int ret=close(Socket);
    if(ret==0) {
        printf("FLNL::Disconnected.\n");
        #ifdef WINDOWS
            WSACleanup();
        #endif
    }
    else {
        printf("FLNL::Error closing connection.\n");
    }

    return ret;
}

//! Thread function waiting for a client connection
//! \param c : A pointer on server object
//! \return NULL
void * accepting(void * c)
{
    server * local_server = (server*)c;

    //Ensure client socket is closed
    close(local_server->Socket);
    local_server->Socket=-1;
    //Wait for new incoming connection
    while(local_server->Socket<0 && local_server->Waiting)
        local_server->Socket = accept(local_server->ServerSocket, NULL, NULL);

    if(!local_server->Waiting)
        return NULL;

    //Connection OK
    local_server->Connected=true;
    printf("FLNL::server::Connected.\n");

    //Create a receiving thread
    if(pthread_create(&local_server->ReceivingThread, NULL, receiving, (void*)local_server)!=0) {
        printf("FLNL::server::Connect() : Error creating receiving thread\n");
        local_server->Connected=false;
    }
    local_server->Waiting=false;

    return NULL;
}
/*#################################################################################################################*/
