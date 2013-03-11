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
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#include "common.h"
#include "utils.h"
#include "tags.h"
#include "assign.h"

static const char *EXT[] = {".MP3", ".APE", ".FLAC", ".OGG", NULL};

//static int check_ext(const char *file, int offset);

static char *
get_line(const char **info_ptr);

//static int get_title(char *title, const char *file);

static char **
count_file(DIR *dp, int *count);

static char *
check_ext(const char *file);

static void
free_files(char **files, int nfile);

static void
free_weights(int **weights, int mat_size);

static int ** 
weight_mat(DIR *dp, AlbumInfo *album, char ***files_ptr, int *mat_size_ptr, int *nfile_ptr);

char *
get_lua_file(const char *server) {
    char *home_path = getenv("HOME");
    size_t file_path_len = strlen(home_path) + strlen("/.fetchtag/");
    size_t default_file_len = strlen(DEFAULT_SERVER_DIR);
    file_path_len = file_path_len < default_file_len ? default_file_len : file_path_len;
    file_path_len += strlen(server) + 5;//server.lua\0

    char *lua_file = (char *)calloc(file_path_len, sizeof(char));
    strcpy(lua_file, home_path);
    strcat(lua_file, "/.fetchtag/");
    strcat(lua_file, server);
    strcat(lua_file, ".lua");
    DBG("%s, %zu\n", lua_file, file_path_len);
    if(!access(lua_file, R_OK)) {
        return lua_file;
    }
    else {
        strcpy(lua_file, DEFAULT_SERVER_DIR); 
        strcat(lua_file, server);
        strcat(lua_file, ".lua");
        if(!access(lua_file, R_OK)) {
            return lua_file;
        }
    }
    free(lua_file);
    return NULL;
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

static char *
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
        free(album->track_title);
    }
    if(album->url) free(album->url);
}


int
update_tag(const char *dir, AlbumInfo *album, bool backup) {
    DIR *dp;
    dp = opendir(dir);
    int total = 0;
    FILE *fp = NULL;
    if(backup) {
        fp = fopen(BACKUP_FILE, "w");
    }

    int nfile, mat_size;
    char **files, *file;
    int **weights; 
    int *assign;
    if(dp) {
        weights = weight_mat(dp, album, &files, &mat_size, &nfile);
        closedir(dp);
        if(weights == NULL) {
            if(fp) fclose(fp);
            return 0;
        }
        
        assign = munkres(weights, mat_size);
        int ntrack = album->num_track;
        char **tracks = album->track_title;
        int i;

        for(i=0; i<ntrack; ++i) {
            if(assign[i] < nfile) {
                file = files[assign[i]];
                if(fp) 
                    backup_tag(file, fp);
                if(set_tag(file, album, i))
                    fprintf(stderr, "error when setting tags for %s\n", file);
                else
                    ++total;
            }
        }
        free_files(files, nfile);
        free_weights(weights, mat_size);
        free(assign);
    }
    else {
        fprintf(stderr, "failed at reading %s: %s\n", dir, strerror(errno));
    }
    if(fp) fclose(fp);
    return total;
}


static char *
check_ext(const char *file) {
    char *dot;
    if((dot=strrchr(file, '.'))==NULL) {
        return NULL;
    }
    const char **ext = EXT;
    while(*ext && strcasecmp(dot, *ext++)!=0) 
        ;
    if(*ext == NULL)
        return NULL;
    else 
        return dot;
}


static char **
count_file(DIR *dp, int *count) {
    struct dirent *ep;
    int nfile = TRACK_GUESS;
    char **file = (char **)malloc(nfile*sizeof(char *));
    char title[MAX_TITLE_LEN];
    int i = 0;

    while((ep = readdir(dp))) {
        if(check_ext(ep->d_name) != NULL) {
            if(i >= nfile) {
                nfile *= 2;
                file = (char **)realloc(file, nfile*sizeof(char *));
            }
            file[i++] = strdup(ep->d_name);
        }
    }
    *count = i;
    if(!i) {//empty dir
        free(file);
        file = NULL;
    }
    return file;
}

static void
free_files(char **array, int size) {
    int i;
    for(i=0; i<size; i++) {
        free(array[i]);
    }
    free(array);
}

static void
free_weights(int **array, int size) {
    int i;
    for(i=0; i<size; i++) {
        free(array[i]);
    }
    free(array);
}


static int **
weight_mat(DIR *dp, AlbumInfo *album, char ***files_ptr, int *mat_size_ptr, int *nfile_ptr) {
    char **files;
    int nfile;
    files = count_file(dp, &nfile);
    *files_ptr = files;
    *nfile_ptr = nfile;
    if(files == NULL) {//empty dir
        return NULL;
    }

    int ntrack = album->num_track;
    if(ntrack == 0) {//empty track
        free_files(files, nfile);
        return NULL;
    }
    int mat_size = nfile > ntrack ? nfile : ntrack;
    *mat_size_ptr = mat_size;
    int **weights = (int **)malloc(mat_size*sizeof(int *));
    char **tracks = album->track_title;

    int i, j, *weight, title_len;
    char *dot;

    init_dp();
    for(i=0; i<nfile; ++i) {
        weight = (int *)malloc(mat_size*sizeof(int));
        dot = strrchr(files[i], '.');
        title_len = dot - files[i];
        for(j=0; j<ntrack; ++j) {
            weight[j] = edit_distance(files[i], title_len, tracks[j], strlen(tracks[j]));
        }
        for(; j<mat_size; ++j) {
            weight[j] = MAX_TITLE_LEN;
        }//dummy tracks if there're more files than tracks
        weights[i] = weight;
    }
    for(; i<mat_size; ++i) {
        weight = (int *)malloc(mat_size*sizeof(int));
        for(j=0; j<mat_size; ++j)
            weight[j] = MAX_TITLE_LEN;
        weights[i] = weight;
    }//dummy file if more tracks than files

    return weights;
}
