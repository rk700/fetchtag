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


#include <stdlib.h>
#include <tag_c.h>
#include "common.h"
#include "tags.h"


int 
id3_set_tag(const char *file, AlbumInfo *album, int index) {
    DBG("change tags: %s\n", file);
    TagLib_File *tag_file;
    //taglib_set_strings_unicode(false);
    if((tag_file=taglib_file_new(file)) == NULL || !taglib_file_is_valid(tag_file)) 
        return -1;

    TagLib_Tag *tag = taglib_file_tag(tag_file);
    taglib_tag_set_title(tag, *(album->track_title+index));
    taglib_tag_set_artist(tag, album->artist);
    DBG("artist %s\n", taglib_tag_artist(tag));
    taglib_tag_set_album(tag, album->album_title);
    taglib_tag_set_track(tag, (unsigned int)index+1);
    taglib_tag_set_year(tag, (unsigned int)album->year);

    taglib_file_save(tag_file);
    taglib_file_free(tag_file);
    return 0;
}
