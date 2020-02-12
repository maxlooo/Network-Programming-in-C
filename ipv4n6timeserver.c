/*
This program is compiled and run on a 64bit Windows 10.
Compiled with MinGW:
gcc listadapters.c -lws2_32
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
        return 0;
    }
    else {
    	printf("WinSock DLL found.\n");
    	printf("Status: %s\n", wsappdata.szSystemStatus);
    }

    printf("Setup and configure local address.\n");
    struct addrinfo preset;
    memset(&preset, 0, sizeof(preset));
    preset.ai_family = AF_INET6;
    preset.ai_socktype = SOCK_STREAM;
    preset.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, "80", &preset, &bind_address);

    printf("Create socket.\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
            bind_address->ai_socktype, bind_address->ai_protocol);
    if (!CHECKVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", WSAGetLastError());
        return 1;
    }

    int option = 0;
    if (setsockopt(socket_listen, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&option, sizeof(option))) {
        fprintf(stderr, "setsockopt() failed. (%d)\n", WSAGetLastError());
        return 1;
    }

    printf("Bind socket to local address.\n");
    if (bind(socket_listen,
                bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", WSAGetLastError());
        return 1;
    }
    freeaddrinfo(bind_address);


    printf("Now listening...\n");
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", WSAGetLastError());
        return 1;
    }


    printf("And waiting for connection...\n");
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    SOCKET socket_client = accept(socket_listen,
            (struct sockaddr*) &client_address, &client_len);
    if (!CHECKVALIDSOCKET(socket_client)) {
        fprintf(stderr, "accept() failed. (%d)\n", WSAGetLastError());
        return 1;
    }


    printf("Client is connected. ");
    char address_buffer[100];
    getnameinfo((struct sockaddr*)&client_address,
            client_len, address_buffer, sizeof(address_buffer), 0, 0,
            NI_NUMERICHOST);
    printf("%s\n", address_buffer);


    printf("Read request.\n");
    char request[1024];
    int bytes_received = recv(socket_client, request, 1024, 0);
    printf("Received %d bytes.\n", bytes_received);
    printf("%.*s", bytes_received, request);


    printf("Send response.\n");
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Local time is: ";
    int bytes_sent = send(socket_client, response, strlen(response), 0);
    printf("Sent %d of %d bytes in HTTP headers and some text.\n", bytes_sent, (int)strlen(response));

    time_t timer;
    time(&timer);
    char *time_msg = ctime(&timer);
    bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
    printf("Sent %d of %d bytes in timer's date and time.\n", bytes_sent, (int)strlen(time_msg));

    printf("Close connection.\n");
    closesocket(socket_client);

    printf("Close listening socket.\n");
    closesocket(socket_listen);

    WSACleanup();
    printf("Finished.\n");
    return 0;
}
