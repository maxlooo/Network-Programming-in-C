#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>

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

    // Allocate memory and get adapter addresses
    DWORD adaptersaddressesbuffersize = 1000;
    PIP_ADAPTER_ADDRESSES adapters;
    do {
        adapters = (PIP_ADAPTER_ADDRESSES)malloc(adaptersaddressesbuffersize);

        if (!adapters) {
            printf("Couldn't allocate %ld bytes for adapters.\n", adaptersaddressesbuffersize);
            WSACleanup();
            return 0;
        }

        int getadaptersaddressesresult = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, 0,
                adapters, &adaptersaddressesbuffersize);
        if (getadaptersaddressesresult == ERROR_BUFFER_OVERFLOW) {
            printf("GetAdaptersAddresses wants %ld bytes for adapters addresses buffer.\n", adaptersaddressesbuffersize);
            free(adapters);
            adapters = NULL;
        } else if (getadaptersaddressesresult == ERROR_SUCCESS) {
            break;
        } else {
            printf("Error from GetAdaptersAddresses: %d\n", getadaptersaddressesresult);
            free(adapters);
            WSACleanup();
            return 0;
        }
    } while (!adapters);


    PIP_ADAPTER_ADDRESSES eachadapter = adapters;
    while (eachadapter) {
        printf("\nAdapter friendly name: %S\n", eachadapter->FriendlyName);

        PIP_ADAPTER_UNICAST_ADDRESS eachaddress = eachadapter->FirstUnicastAddress;
        while (eachaddress) {
            printf("\t%s",
                    eachaddress->Address.lpSockaddr->sa_family == AF_INET ?
                    "IPv4" : "IPv6");

            char ap[100];

            getnameinfo(eachaddress->Address.lpSockaddr,
                    eachaddress->Address.iSockaddrLength,
                    ap, sizeof(ap), 0, 0, NI_NUMERICHOST);
            printf("\t%s\n", ap);

            eachaddress = eachaddress->Next;
        }

        eachadapter = eachadapter->Next;
    }

    free(adapters);
    WSACleanup();
    return 0;
}
