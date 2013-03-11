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

#ifndef UTILS_H
#define UTILS_H
#include "common.h"

char *
get_lua_file(const char *server);

void
parse_album(AlbumInfo *album, const char *info);

char **
parse_track(char *track_line, int *num_track);

void
print_album(AlbumInfo *album);

void
free_album(AlbumInfo *album);

//int assign(const char *file, AlbumInfo *album, FILE *fp);

int
update_tag(const char *dir, AlbumInfo *album, bool backup);

//int ** weight_mat(const char *dir, AlbumInfo *album, char ***files_ptr, int *mat_size_ptr, int *nfile_ptr);

//void pair_dis(const char *dir, AlbumInfo *album);
#endif
