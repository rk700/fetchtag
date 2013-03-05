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

#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>

#define MAX_STRING_LEN 1000

#define ERROR(...) \
do{ \
    fprintf(stderr, "[ERROR]%s %s(Line %d): ",__FILE__,__FUNCTION__,__LINE__); \
    fprintf(stderr, __VA_ARGS__); \
}while(0)
    
   
#define INFO(...) \
do{ \
    fprintf(stderr, "[INFO]%s %s(Line %d): ",__FILE__,__FUNCTION__,__LINE__); \
    fprintf(stderr, __VA_ARGS__); \
}while(0)
    
    
#ifdef DEBUG    
#define DBG(...) \
do{ \
    fprintf(stdout, "[DEBUG]%s %s(Line %d): ",__FILE__,__FUNCTION__,__LINE__); \
    fprintf(stdout, __VA_ARGS__); \
}while(0)
#else
#define DBG(...)
#endif

typedef enum {false, true} bool;

extern const char *DEFAULT_SERVER;
extern const char *DEFAULT_SERVER_DIR;

typedef struct AlbumInfo_ {
    int num_track;
    char *publisher;
    char *artist;
    char *album_title;
    char *url;
    unsigned int year;
    char **track_title;
} AlbumInfo;
#endif
