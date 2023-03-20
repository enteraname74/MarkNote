/* marknote-window.h
 *
 * Copyright 2023 Noah  Penin
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <adwaita.h>
#include "marknote-types.h"

G_BEGIN_DECLS

typedef struct FileInfoStruct FileList;

struct FileInfoStruct {
  gboolean is_new_file;
  GFile *file;
  struct FileInfoStruct *next_file;
};

void show_file_list_infos(FileList *list);
FileList * file_list_add_file(FileList *list, GFile *new_file, gboolean is_new_file);
FileList * file_list_delete_at(FileList *list, int pos);

FileList * file_list_get_file_info_from_pos(FileList *list, int pos);
FileList * file_list_get_file_infos_from_tab_view(FileList *list, AdwTabView *tab_view);
gboolean file_list_search_file(FileList *list, GFile *file);
int file_list_get_length(FileList *list);
int file_list_get_pos_of_file_from_path(FileList *list, char * path);

G_END_DECLS
