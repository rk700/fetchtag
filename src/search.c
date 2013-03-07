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

#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h> 

#include "common.h"
#include "utils.h"
#include "search.h"

int
get_lua(const char *file, lua_State **L) {
    if((*L = luaL_newstate()) == NULL)
        return -1;
    luaL_openlibs(*L);

    if(luaL_loadfile(*L, file) || lua_pcall(*L, 0, LUA_MULTRET, 0)) {
        lua_close(*L);
        return -1;
    }
    return 0;
}


int
get_host(lua_State *L, char **host, char **port) {
    lua_getglobal(L, "host");
    lua_getglobal(L, "port");
    if(!lua_isstring(L, -2) || !lua_isstring(L, -1))
        return -1;
    *host = strdup(lua_tostring(L, -2));
    *port = strdup(lua_tostring(L, -1));
    lua_pop(L, 2);
    return 0;
}

int
get_query(lua_State *L, char **query, const char *artist, const char *album) {
    lua_getglobal(L, "generateRequest");
    if(!lua_isfunction(L, -1)) {
        return -1;
    }
    lua_newtable(L);
    if(artist) table_add(L, "artist", artist);
    if(album) table_add(L, "album", album);
    lua_call(L, 1, 1);
    if(!lua_isstring(L, -1)) {
        return -1;
    }
    *query = strdup(lua_tostring(L, -1));
    lua_pop(L, 1);
    return 0;
}
 
void
table_add(lua_State *L, const char *key, const char *value) {
    lua_pushstring(L, key);
    lua_pushstring(L, value);
    lua_rawset(L, -3);
}

AlbumInfo *
get_results(lua_State *L, char *file, int *num_albums) {
    lua_getglobal(L, "parseResult");
    if(!lua_isfunction(L, -1)) {
        *num_albums = 0;
        fprintf(stderr, "no function \"parseResult\" in Lua script\n");
        return NULL;
    }
    lua_pushstring(L, file);
    lua_call(L, 1, 1);
    lua_pushnil(L);
    int t = -2;
    if(lua_next(L, t) == 0) {
        *num_albums = 0;
        fprintf(stderr, "function \"parseResult\" failed\n");
        return NULL;
    }
    if(!lua_isnumber(L, -1)) {
        *num_albums = 0;
        fprintf(stderr, "cannot get number of search results\n");
        return NULL;
    }
    *num_albums = (int)lua_tonumber(L, -1);
    AlbumInfo *albums = (AlbumInfo *)malloc(sizeof(AlbumInfo)*(*num_albums));
    AlbumInfo *albums_ptr = albums;
    lua_pop(L, 1);

    while(lua_next(L, t) != 0) {
        if(!lua_isstring(L, -1)) {
            *num_albums = 0;
            fprintf(stderr, "error when parsing results info\n");
            free(albums);
            return NULL;
        }
        parse_album(albums_ptr++, lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    return albums;
}
