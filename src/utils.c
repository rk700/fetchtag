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


#define _GNU_SOURCE
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#include "utils.h"
#include "common.h"
#include "tags.h"

const char *EXT[] = {".MP3", ".APE", ".FLAC", ".OGG", NULL};

int
get_lua_file(char **lua_file, const char *server) {
    char *home_path = getenv("HOME");
    size_t server_path_len = strlen(home_path) + strlen("/.fetchtag/");
    size_t defalut_dir_len = strlen(DEFAULT_SERVER_DIR);
    server_path_len = server_path_len < defalut_dir_len ? defalut_dir_len : server_path_len;
    server_path_len += strlen(server) + 5;//server.lua\0

    char *server_path = (char *)malloc(sizeof(char)*server_path_len);
    memset(server_path, 0, sizeof(char)*server_path_len);
    strcpy(server_path, home_path);
    strcat(server_path, "/.fetchtag/");
    strcat(server_path, server);
    strcat(server_path, ".lua");
    DBG("%s, %zu\n", server_path, server_path_len);
    if(!access(server_path, R_OK)) {
        *lua_file = server_path;
        return 0;
    }
    else {
        strcpy(server_path, DEFAULT_SERVER_DIR); 
        strcat(server_path, server);
        strcat(server_path, ".lua");
        if(!access(server_path, R_OK)) {
            *lua_file = server_path;
            return 0;
        }
    }
    free(server_path);
    return -1;
}

/*
 * parse album:
 * make sure that the Lua stript contains a function called "parseResult",
 * which takes the result file name as argument, 
 * and returns a table like: {NUM_RESULTS, ALBUM_1, ALBUM_2, ...}
 * NUM_RESULTS is an int showing number of albums in the search results
 * ALBUM_1, ALBUM_2, ... are album infomation; each of them is a string
 * each album info string has the following form:
 * ===== start ====
 * PUBLISHER
 * ARTIST
 * YEAR
 * ALBUM_TITLE
 * TRACK_TITLE
 * URL
 *
 * ===== end ======
 * each of these fields occupies an entire line
 * TRACK_TITLE contains titles of all tracks, deliminated by tab
 * the first token of TRACK_TITLE is the number of tracks
 * there's a newline after URL
 */

void
parse_album(AlbumInfo *album, const char *info) {
    album->publisher = get_line(&info);
    album->artist = get_line(&info);

    char *year = get_line(&info);
    if(year) album->year = (unsigned int)atoi(year);
    else album->year = 0;
    free(year);

    album->album_title = get_line(&info);

    char *tracks = get_line(&info);    
    album->track_title = parse_track(tracks, &(album->num_track));
    free(tracks);

    album->url = get_line(&info);
}

char *
get_line(const char **info_ptr) {
    const char *info = *info_ptr;
    if(*info == '\n') {
        *info_ptr = info+1;
        return NULL;
    }
    const char *end = info;
    while(*end != '\n')
        end++;
    char *line = strndup(info, end-info+1);
    line[end-info] = '\0';
    *info_ptr = end+1;
    return line;
}


char **
parse_track(char *track_line, int *num_track) {
    if(!track_line) {
        *num_track = 0;
        return NULL;
    }
    *num_track = atoi(strtok(track_line, "\t"));
    char **track_title = (char **)malloc(sizeof(char *)*(*num_track));
    char **tracks = track_title;
    char *track;
    while((track=strtok(NULL, "\t")) != NULL) {
        *tracks++ = strdup(track);
    }
    return track_title;
}


void
print_album(AlbumInfo *album) {
    printf("Publisher: %s\n", album->publisher ? album->publisher : "N/A");
    printf("Artist: %s\n", album->artist ? album->artist : "N/A");

    printf("Year: ");
    if(album->year) printf("%u\n", album->year);
    else printf("N/A\n");

    printf("Album: %s\n", album->album_title ? album->album_title : "N/A");
    printf("Number of tracks: %d\n", album->num_track);
    printf("Tracks:\n");
    char **track_title = album->track_title;
    int i;
    for(i=0; i<album->num_track; i++) {
        printf("\t%2d. %s\n", i+1, *track_title++);
    }
    
    printf("URL: %s\n", album->url ? album->url : "N/A");
}
    
void
free_album(AlbumInfo *album) {
    if(album->publisher) free(album->publisher);
    if(album->artist) free(album->artist);
    if(album->album_title) free(album->album_title);
    if(album->track_title) {
        int num_track = album->num_track;
        char **track_title = album->track_title;
        while(num_track-- > 0) {
            free(*track_title++);
        }
    }
    if(album->url) free(album->url);
}

int
assign(const char *file, AlbumInfo *album) {
    int i;
    const char *track_title;
    const char *substring;
    DBG("file is %s\n", file);
    for(i=0; i<album->num_track; i++) {
        track_title = *(album->track_title+i);
        if((substring=strcasestr(file, track_title))!=NULL && check_ext(substring, strlen(track_title))) {
            if(id3_set_tag(file, album, i))
                ERROR("error when settign tags for %s\n", file);
            else
                return 1;
        }
    }
    return 0;
}

int 
check_ext(const char *file, int offset) {
    const char **ext = EXT;
    const char *fext = file+offset;
    DBG("check ext %s, %s\n", file, fext);
    while(*ext) {
        if(strcasestr(fext, *ext) && strlen(fext)==strlen(*ext)) {
            return 1;
        }
        ext++;
    }
    return 0;
}

int
update_tag(const char *dir, AlbumInfo *album) {
    DIR *dp;
    dp = opendir(dir);
    struct dirent *ep;
    int total = 0;

    if(dp) {
        while((ep = readdir(dp))) {
            total += assign(ep->d_name, album);
        }
        closedir(dp);
    }
    else {
        ERROR("failed at opening the directory %s\n", dir);
    }
    return total;
}

