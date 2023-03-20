/* marknote-window.c
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

#include "marknote-file-list.h"

void show_file_list_infos(FileList *list)
{
  FileList *temp = list;
  char *file_name;

  g_print("------- START FILE LIST --------\n");

  if (list == NULL) {
    g_print("NO ELEMENTS IN LIST\n");
  } else {
    while(temp != NULL)
      {

        if (temp->file != NULL) {
          file_name = g_file_get_basename (temp->file);
        } else {
          file_name = "Untitled Document";
        }
        g_print("%s\n", file_name);

        if (temp->is_new_file == true)
          {
            g_print("new file\n");
          }
        else
          {
            g_print("not new file\n");
          }
        g_print("-------------\n");
        temp = temp->next_file;
      }
  }
  g_print("------- END FILE LIST --------\n");
  g_print("\n");
}

FileList * file_list_add_file(FileList *list, GFile *new_file, gboolean is_new_file)
{
  FileList *new_elt = malloc(sizeof(*new_elt));
  FileList *temp = list;

  new_elt->is_new_file = is_new_file;
  new_elt->next_file = NULL;
  new_elt->file = new_file;

  if (list == NULL) {
    return new_elt;
  } else {
    while(temp->next_file != NULL)
      {
        g_print("iter\n");
        temp=temp->next_file;
      }
    temp->next_file = new_elt;
  }
  return list;
}

FileList * file_list_delete_at(FileList *list, int pos)
{
  FileList *temp = list;

  if (pos == 0)
    {
      g_print("pos == 0\n");
      return list->next_file;
    }
  else
    {
      for (int i = 0; i < pos - 1; i++)
        {
          temp=temp->next_file;
        }
      temp->next_file = temp->next_file->next_file;
      return list;
    }
}

FileList * file_list_get_file_info_from_pos(FileList *list, int pos)
{
  FileList *temp = list;

  for (int i = 0; i < pos; i++)
    {
      temp=temp->next_file;
    }
  return temp;
}

int file_list_get_length(FileList *list)
{
  FileList *temp = list;
  int length = 0;

  while (temp != NULL) {
    length++;
    temp = temp->next_file;
  }

  return length;
}

FileList * file_list_get_file_infos_from_tab_view(FileList *list, AdwTabView *tab_view)
{
  AdwTabPage *current_page = adw_tab_view_get_selected_page (tab_view);
  int page_pos = adw_tab_view_get_page_position (tab_view, current_page);
  return file_list_get_file_info_from_pos(list, page_pos);
}

gboolean file_list_search_file(FileList *list, GFile *file)
{
  FileList *temp = list;

  while(temp != NULL)
    {
      if (!(temp->is_new_file))
        {
          if (strcmp(
            g_file_get_path (file), g_file_get_path (temp->file)) == 0)
            {
              return true;
            }
        }
      temp=temp->next_file;
    }

  return false;
}

int file_list_get_pos_of_file_from_path(FileList *list, char * path)
{
  FileList *temp = list;
  int i = 0;

  while(temp != NULL)
    {
      if (!(temp->is_new_file))
        {
          if (strcmp(
            path, g_file_get_path (temp->file)) == 0)
          {
            return i;
          }
        }
      i++;
      temp=temp->next_file;
    }

  return -1;
}
