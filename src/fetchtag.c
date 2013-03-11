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


#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <lua.h>

#include "search.h"
#include "common.h"
#include "connect.h"
#include "utils.h"
#include "assign.h"


const char *DEFAULT_SERVER = "/usr/share/fetchtag/douban.lua";
const char *DEFAULT_SERVER_DIR = "/usr/share/fetchtag/";
const char *SEARCH_RES = ".fetchtag_res";
const char *BACKUP_FILE = ".fetchtag_bak";


static struct option long_opts[] =
{
    { "artist", 1, 0, 'a' },
    { "album", 1, 0, 'b' },
    { "help", 0, 0, 'h' },
    { "server", 1, 0, 's' },
    { "recover", 0, 0, 'r' },
    { "nobackup", 0, 0, 'k' },
    { 0, 0, 0, 0 }
};


static char *artist = NULL;
static char *album = NULL;
static char *server = NULL;
static bool backup = true;
static bool recover = false;
static lua_State *L = NULL;
static char *lua_file = NULL;
static char *port = NULL;
static char *host = NULL;
static char *query = NULL; 
static int num_albums = 0;
static AlbumInfo *albums = NULL;


static void
show_usage(int exit_status) {
    printf("\nUsage:\nfetchtag OPTIONS [DIR]\n"
            "Search album and update tags for each song.\n"
            "Example: fetchtag -a jay\\ chou -b fantasy\n\n"
            "Options:\n"
            "%-20s\t%s\n"
            "%-20s\t%s\n"
            "%-20s\t%s\n"
            "%-20s\t%s\n"
            "%-20s\t%s\n"
            "%-20s\t%s\n"
            "%-20s\t%s\n\n"
            "Use DIR to specify the directory containg audio files you want to update.\n"
            "If omitted, the current working directory will be used.\n",
            "-a, --artist=ARTIST", "specify the artist",
            "-b, --album=ALBUM", "specify the album",
            "-s, --server=SERVER", "specify the server for seaching; ", 
            "", "default server(douban) is used if omitted",
            "-k, --nobackup", "do not backup tags when updating",
            "-r, --recover", "recover tags",
            "-h, --help", "show this help"
            );
    exit(exit_status);
}

static void
get_parameters(int argc, char *argv[]) {
    int opt;
    while ((opt=getopt_long(argc, argv, "a:b:hs:rk", long_opts, NULL)) != -1) {
        switch (opt) {
        case 'a':
            artist = strdup(optarg);
            break;
        case 'b':
            album = strdup(optarg);
            break;
        case 's':
            server = strdup(optarg);
            break;
        case 'h':
            show_usage(EXIT_SUCCESS);
            break;
        case 'r':
            recover = true;
            break;
        case 'k':
            backup = false;
            break;
        default:
            show_usage(EXIT_FAILURE);
            break;
        }
    }
}

static void
cleanup(int status) {
    free(artist);
    free(album);
    free(server);
    if(L) lua_close(L);
    free(lua_file);
    free(host);
    free(port);
    free(query);
    int i;
    for(i=0; i<num_albums; i++) {
        free_album(albums+i);
    }
    free(albums);

    exit(status);
}

   

int
main(int argc, char *argv[]) {
    get_parameters(argc, argv);

    char *dir = argc > optind ? *(argv+optind) : ".";

    if(chdir(dir)) {
        fprintf(stderr, "error with %s: %s\n", dir, strerror(errno));
        cleanup(EXIT_FAILURE);
    }
    if(recover) {
        cleanup(recover_tag());
    }


    if(!artist && !album) {
        fprintf(stderr, "no search field\n");
        cleanup(EXIT_FAILURE);
    }

    DBG("after get parameters\n");

    if(server) {
        DBG("server not null\n");
        if((lua_file=get_lua_file(server))==NULL) {
            fprintf(stderr, "cannot access file %s.lua: %s\n", server, strerror(errno));
            cleanup(EXIT_FAILURE);
        }
    }
    else {
        server = strdup("douban");
        lua_file = strdup(DEFAULT_SERVER);
    }
    DBG("after get lua_file\n");


    if((L=get_lua(lua_file))==NULL) {
        fprintf(stderr, "error at loading %s\n", lua_file);
        cleanup(EXIT_FAILURE);
    }

    DBG("after get lua state\n");

    if(get_host(L, &host, &port)) {
        fprintf(stderr, "cannot get host and port\n");
        cleanup(EXIT_FAILURE);
    }

    DBG("after get host and port\n");

    if((query=get_query(L, artist, album))==NULL) {
        fprintf(stderr, "error when calling function \"generateRequest\" in %s.lua\n", server);
        cleanup(EXIT_FAILURE);
    }
    DBG("after get query\n");

    printf("searching ... ");
    fflush(stdout);
#if 1
    int sockfd;
    if((sockfd=setup_connection(host, port))==0 || send_query(sockfd, query)<0) {
        cleanup(EXIT_FAILURE);
    }

    if(write_to_file(sockfd) < 0) {
        cleanup(EXIT_FAILURE);
    }
    close(sockfd);
#endif

    printf("done!\n----------------------------------------------------\n");

    int num_albums;
    AlbumInfo *albums = get_results(L, &num_albums);
    lua_close(L);
    L = NULL;

#if 1
    int i;
    for(i=0; i<num_albums; i++) {
        printf("%2d %s/%s\n", i+1, albums[i].artist, albums[i].album_title);
    }
    printf("----------------------------------------------------\n"
           "==> enter NUMBER to choose an album\n"
           "==> enter \"v NUMBER\" to show details of the album\n"
           "==> enter \"l\" to list the search results\n"
           "==> ");
    char *input = NULL;
    size_t input_size;
    while(getline(&input, &input_size, stdin) != -1) {
        if(input_size > 1) {
            if(*input == 'l') {
                printf("----------------------------------------------------\n");
                for(i=0; i<num_albums; i++) 
                    printf("%2d %s/%s\n", i+1, albums[i].artist, albums[i].album_title);
                printf("----------------------------------------------------\n");
            }
            else if(*input == 'v') {
                int idx = atoi(input+1)-1;
                if(idx>=0 && idx<num_albums) {
                    print_album(albums+idx);
                }
                else
                    printf("Out of bound! Pleast put a number between 0 and %d\n", num_albums);
            }
            else if(isdigit(*input)) {
                int idx = atoi(input)-1;
                if(idx>=0 && idx<num_albums) {
                    printf("updating... ");
                    printf("done! %d files updated.\n", update_tag(dir, albums+idx, backup));
                    break;
                }
                else
                    printf("Out of bound! Pleast put a number between 0 and %d\n", num_albums);
            }
        }
        printf("==> ");
    }

#endif
    unlink(SEARCH_RES);

    cleanup(EXIT_SUCCESS);
}
