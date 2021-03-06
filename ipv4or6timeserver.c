/*
This program is compiled and run on a 64bit Windows 10.
Compiled with MinGW:
gcc ipv4or6timeserver.c -lws2_32
For Visual Studio, these lines of code are needed:
#pragma comment(lib, "ws2_32.lib")
*/
#include <winsock2.h>
#include <ws2tcpip.h>
#define CHECKVALIDSOCKET(s) ((s) != INVALID_SOCKET)

#include <stdio.h>
#include <string.h>
#include <time.h>

int main() {

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

    printf("Setup and configure local address.\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bindthisaddress;
    getaddrinfo(0, "80", &hints, &bindthisaddress);

    printf("Create socket.\n");
    SOCKET socketlistener;
    socketlistener = socket(bindthisaddress->ai_family,
            bindthisaddress->ai_socktype, bindthisaddress->ai_protocol);
    if (!CHECKVALIDSOCKET(socketlistener)) {
        fprintf(stderr, "socket() failed. (%d)\n", WSAGetLastError());
        return 1;
    }

    // set dual stack sockets, enables reading of both IPV4 and V6 address
    int option = 0;
    if (setsockopt(socketlistener, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&option, sizeof(option))) {
        fprintf(stderr, "setsockopt() failed. (%d)\n", WSAGetLastError());
        return 1;
    }

    printf("Bind socket to local address.\n");
    if (bind(socketlistener,
                bindthisaddress->ai_addr, bindthisaddress->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", WSAGetLastError());
        return 1;
    }
    freeaddrinfo(bindthisaddress);

    printf("Now listening...\n");
    if (listen(socketlistener, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", WSAGetLastError());
        return 1;
    }

    printf("And waiting for connection...\n");
    struct sockaddr_storage client_address;
    socklen_t clientaddresslength = sizeof(client_address);
    printf("Program is now waiting at accept function...\n");
    SOCKET socketforclient = accept(socketlistener,
            (struct sockaddr*) &client_address, &clientaddresslength);
    if (!CHECKVALIDSOCKET(socketforclient)) {
        fprintf(stderr, "accept() failed. (%d)\n", WSAGetLastError());
        return 1;
    }

    printf("Client is connected. ");
    char FQDNhostname[100];
    getnameinfo((struct sockaddr*)&client_address,
            clientaddresslength, FQDNhostname, sizeof(FQDNhostname), 0, 0,
            NI_NUMERICHOST);
    
    printf("%s\n", FQDNhostname);

    printf("Read request.\n");
    char request[1024];
    int bytes_received = recv(socketforclient, request, 1024, 0);
    printf("Received %d bytes.\n", bytes_received);
    printf("%.*s", bytes_received, request);

    printf("Send response.\n");
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Local time is: ";
    int bytes_sent = send(socketforclient, response, strlen(response), 0);
    printf("Sent %d of %d bytes in HTTP headers and some text.\n", bytes_sent, (int)strlen(response));

    time_t timer;
    time(&timer);
    char *time_msg = ctime(&timer);
    bytes_sent = send(socketforclient, time_msg, strlen(time_msg), 0);
    printf("Sent %d of %d bytes in timer's date and time.\n", bytes_sent, (int)strlen(time_msg));

    printf("Close connection.\n");
    closesocket(socketforclient);

    printf("Close listening socket.\n");
    closesocket(socketlistener);

    WSACleanup();
    printf("WSACleanup done.\n");
    return 0;
}
