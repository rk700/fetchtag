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
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "connect.h"
#include "common.h"

int
setup_connection(const char *host, int port) {
    int sockfd = 0;
    struct hostent *hostent;
    struct sockaddr_in sockaddr;

    if (!host || port <= 0)
        return 0;

    if((hostent=gethostbyname(host)) == NULL) {
        ERROR("cannot resolve host %s\n", host);
        return 0;
    }

    memset((void *)&sockaddr, 0, sizeof(sockaddr));
    memcpy(&sockaddr.sin_addr.s_addr, *(hostent->h_addr_list), sizeof(sockaddr.sin_addr.s_addr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);

    if((sockfd=socket(AF_INET,SOCK_STREAM,0)) < 0) {
        ERROR("cannot create socket\n");
        return 0;
    }
    if(connect(sockfd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)) < 0) {
        ERROR("cannot connect to %s\n", host);
        return 0;
    }

    return sockfd;
}


int
send_query(int sockfd, const char *query) {
    int bytes_written;
    if((bytes_written=send(sockfd, query, strlen(query)+1, 0)) < 0) {
        ERROR("error sending query\n");
        return -1;
    }
    return 0;
}

int
write_to_file(int sockfd, const char *file) {
    FILE *fp;
    if((fp=fopen(file, "w")) == NULL) {
        ERROR("cannot write file %s\n", file);
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
        ERROR("error recv result\n");
        return -1;
    }
    return 0;
}
