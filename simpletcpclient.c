/*
This program is compiled and run on a 64bit Windows 10.
Compiled with MinGW:
gcc simpletcpclient.c -lws2_32
For Visual Studio, these lines of code are needed:
#pragma comment(lib, "ws2_32.lib")
*/
#include <winsock2.h>
#include <ws2tcpip.h>
#define CHECKVALIDSOCKET(s) ((s) != INVALID_SOCKET)

#include <stdio.h>
#include <string.h>
#include <conio.h>

int main(int argc, char *argv[]) {

    if (argc < 3) {
        fprintf(stderr, "usage: %s hostname port\n", argv[0]);
        fprintf(stderr, "example: %s 127.0.0.1 80\n", argv[0]);
        return 1;
    }

    // Start the WinSock Application and check errors
    WSADATA wsappdata;
    WORD wsappversion;
    int wsapperror;

    wsappversion = MAKEWORD(2, 2);
    wsapperror = WSAStartup(wsappversion, &wsappdata);

    if (wsapperror) {
        printf("Could not find usable WinSock DLL.\n");
        return 1;
    }
    else {
    	printf("WinSock DLL found.\n");
    	printf("Status: %s\n", wsappdata.szSystemStatus);
    }

    printf("Configure remote server address.\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *server_address;
    if (getaddrinfo(argv[1], argv[2], &hints, &server_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", WSAGetLastError());
        return 1;
    }


    printf("Remote server address is: ");
    char serveraddressbuffer[100];
    char portservicebuffer[100];
    getnameinfo(server_address->ai_addr, server_address->ai_addrlen,
            serveraddressbuffer, sizeof(serveraddressbuffer),
            portservicebuffer, sizeof(portservicebuffer),
            NI_NUMERICHOST);
    printf("%s %s\n", serveraddressbuffer, portservicebuffer);


    printf("Create socket to server.\n");
    SOCKET socket2server;
    socket2server = socket(server_address->ai_family,
            server_address->ai_socktype, server_address->ai_protocol);
    if (!CHECKVALIDSOCKET(socket2server)) {
        fprintf(stderr, "socket() failed. (%d)\n", WSAGetLastError());
        return 1;
    }


    printf("Connecting...\n");
    if (connect(socket2server,
                server_address->ai_addr, server_address->ai_addrlen)) {
        fprintf(stderr, "connect() failed. (%d)\n", WSAGetLastError());
        return 1;
    }
    freeaddrinfo(server_address);

    printf("Connected.\n");
    printf("To send data, enter text followed by enter.\n");

    while(1) {

        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket2server, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socket2server+1, &reads, 0, 0, &timeout) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", WSAGetLastError());
            return 1;
        }

        if (FD_ISSET(socket2server, &reads)) {
            char read[4096];
            int bytes_received = recv(socket2server, read, 4096, 0);
            if (bytes_received < 1) {
                printf("Connection closed by peer.\n");
                break;
            }
            printf("Received (%d bytes): %.*s",
                    bytes_received, bytes_received, read);
        }

        if(_kbhit()) {
            char read[4096];
            if (!fgets(read, 4096, stdin)) break;
            printf("Sending: %s", read);
            int bytes_sent = send(socket2server, read, strlen(read), 0);
            printf("Sent %d bytes.\n", bytes_sent);
        }
    } //end while(1)

    printf("Closing socket...\n");
    closesocket(socket2server);
    WSACleanup();
    printf("Finished.\n");
    return 0;
}

