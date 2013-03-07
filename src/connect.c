/*
 * Copyright (C) 2013 Ruikai Liu <lrk700@gmail.com>
 *
 * This file is part of fetchtag.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rbook.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "connect.h"
#include "common.h"

int
setup_connection(const char *host, const char *port) {
    int sockfd = 0;
    struct hostent *hostent;
    struct sockaddr_in sockaddr;
    struct addrinfo hints, *res, *ressave;
    int error;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if((error=getaddrinfo(host, port, &hints, &res))) {
        fprintf(stderr, "cannot resolve host: %s\n", gai_strerror(error));
        return 0;
    }
    ressave = res;

    do {
        sockfd = socket(AF_INET, SOCK_STREAM, res->ai_protocol);
        if(sockfd < 0)
            continue;
        if(connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            break;
        close(sockfd);
    } while((res=res->ai_next) != NULL);
    
    if(res == NULL) {
        fprintf(stderr, "connect error for %s, %s\n", host, port);
        return 0;
    }
    
    freeaddrinfo(ressave);
    return sockfd;
}


int
send_query(int sockfd, const char *query) {
    int bytes_written;
    if((bytes_written=send(sockfd, query, strlen(query)+1, 0)) < 0) {
        fprintf(stderr, "send error: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int
write_to_file(int sockfd, const char *file) {
    FILE *fp;
    if((fp=fopen(file, "w")) == NULL) {
        fprintf(stderr, "cannot write file %s: %s\n", file, strerror(errno));
        return -1;
    }
    char recvline[MAX_STRING_LEN+1];
    int bytes_read;
    while((bytes_read=recv(sockfd, (void *)&recvline, MAX_STRING_LEN, 0)) > 0) {
        recvline[bytes_read] = 0;
        fwrite(recvline, bytes_read, 1, fp);
    }
    fclose(fp);
    if(bytes_read < 0) {
        fprintf(stderr, "cannot recv data: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}
