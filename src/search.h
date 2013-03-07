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

#ifndef SEARCH_H
#define SEARCH_H
#include "common.h"

int
get_lua(const char *file, lua_State **L);

int
get_host(lua_State *L, char **host, char **port);

int
get_query(lua_State *L, char **query, const char *artist, const char *album);

void
table_add(lua_State *L, const char *key, const char *value);

AlbumInfo *
get_results(lua_State *L, char *file, int *num_albums);


#endif
