#include "bofdefs.h"
#include "beacon.h"
#include "base.c"

char* next(char* string, char delim, char* out) {
  char* pos = MSVCRT$strchr(string, delim);
  size_t length = pos - string;
  MSVCRT$memcpy(out, string, length);
  out[length] = '\0';
  return pos + 1;
}

void scan(char* host, char port[8]) {
  struct addrinfo* result = NULL;
  struct addrinfo hints;

  intZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  if (WS2_32$getaddrinfo(host, port, &hints, &result) == ERROR_SUCCESS) {
    SOCKET sock = WS2_32$socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock) {
      u_long nonblock = 1;
      WS2_32$ioctlsocket(sock, FIONBIO, &nonblock);

      struct timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;

      struct fd_set sockets;
      FD_ZERO(&sockets);
      FD_SET(sock, &sockets);

      WS2_32$connect(sock, result->ai_addr, result->ai_addrlen);
      WS2_32$select(1, NULL, &sockets, NULL, &tv);

      if (WS2_32$__WSAFDIsSet(sock, &sockets)) {
        BeaconPrintf(CALLBACK_OUTPUT, "%s:%s\n", host,port);
      }

      WS2_32$closesocket(sock);
    }
    WS2_32$freeaddrinfo(result);
  }
}

void go(char* args, int len) {
  datap parser;

  char* hosts;
  char* host;

  char* ports;
  char port[8];

  char oct1[8];
  char oct2[8];
  char oct3[8];
  char oct4[8];
  char oct5[8];
  char base[64];

  int istart = 0;
  int istop = 0;

  int pstart = 0;
  int pstop = 0;

  int ilist = 0;
  int irange = 0;

  int plist = 0;
  int prange = 0;

  BeaconDataParse(&parser, args, len);
  hosts = BeaconDataExtract(&parser, NULL);
  ports = BeaconDataExtract(&parser, NULL);

  // Get hosts
  if (MSVCRT$strchr(hosts, ':')) {
    irange = 1;
    hosts = next(hosts, '.', oct1);
    hosts = next(hosts, '.', oct2);
    hosts = next(hosts, '.', oct3);
    hosts = next(hosts, ':', oct4);
    MSVCRT$strcpy(oct5, hosts);
    istart = MSVCRT$atoi(oct4);
    istop = MSVCRT$atoi(oct5);
  } else if (MSVCRT$strchr(hosts, ',')) {
    ilist = 1;
    hosts = next(hosts, '.', oct1);
    hosts = next(hosts, '.', oct2);
    hosts = next(hosts, '.', oct3);
  } else {
    host = hosts;
  }

  // Get ports
  if (MSVCRT$strchr(ports, ':')) {
    prange = 1;
    char first[8];
    char last[8];
    MSVCRT$strcpy(first, MSVCRT$strtok(ports, ":"));
    MSVCRT$strcpy(last, MSVCRT$strtok(NULL, ":"));
    pstart = MSVCRT$atoi(first);
    pstop = MSVCRT$atoi(last);
  } else if (MSVCRT$strchr(ports, ',')) {
    plist = 1;
  } else {
    MSVCRT$strcpy(port, ports);
  }

  // Scan
  if (irange) {
    for (int h = istart; h <= istop; h++) {
      MSVCRT$sprintf(oct4, "%d", h);
      MSVCRT$sprintf(base, "%s.%s.%s.%s", oct1,oct2,oct3,oct4);
      host = base;

      if (prange) {
        for (int p = pstart; p <= pstop; p++) {
          MSVCRT$sprintf(port, "%d", p);
          scan(host, port);
        }
      } else if (plist) {
        while (MSVCRT$strchr(ports, ',')) {
          ports = next(ports, ',', port);
          scan(host, port);
        }
        MSVCRT$strcpy(port, ports);
        scan(host, port);
      } else {
        scan(host, port);
      }
    }
  } else if (ilist) {
    while (MSVCRT$strchr(hosts, ',')) {
      hosts = next(hosts, ',', oct4);
      MSVCRT$sprintf(base, "%s.%s.%s.%s", oct1,oct2,oct3,oct4);
      host = base;

      if (prange) {
        for (int p = pstart; p <= pstop; p++) {
          MSVCRT$sprintf(port, "%d", p);
          scan(host, port);
        }
      } else if (plist) {
        while (MSVCRT$strchr(ports, ',')) {
          ports = next(ports, ',', port);
          scan(host, port);
        }
        MSVCRT$strcpy(port, ports);
        scan(host, port);
      } else {
        scan(host, port);
      }
    }

    MSVCRT$strcpy(oct4, hosts);
    MSVCRT$sprintf(base, "%s.%s.%s.%s", oct1,oct2,oct3,oct4);
    host = base;

    if (prange) {
      for (int p = pstart; p <= pstop; p++) {
        MSVCRT$sprintf(port, "%d", p);
        scan(host, port);
      }
    } else if (plist) {
      while (MSVCRT$strchr(ports, ',')) {
        ports = next(ports, ',', port);
        scan(host, port);
      }
      MSVCRT$strcpy(port, ports);
      scan(host, port);
    } else {
      scan(host, port);
    }
  } else {
    if (prange) {
      for (int p = pstart; p <= pstop; p++) {
        MSVCRT$sprintf(port, "%d", p);
        scan(host, port);
      }
    } else if (plist) {
      while (MSVCRT$strchr(ports, ',')) {
        ports = next(ports, ',', port);
        scan(host, port);
      }
      MSVCRT$strcpy(port, ports);
      scan(host, port);
    } else {
      scan(host, port);
    }
  }
}
