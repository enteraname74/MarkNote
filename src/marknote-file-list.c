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
#include <uuid/uuid.h>

void show_file_list_infos(FileList *list)
{
  FileList *temp = list;

  g_print("------- START FILE LIST --------\n");

  if (list == NULL) {
    g_print("NO ELEMENTS IN LIST\n");
  } else {
    while(temp != NULL)
      {
        file_list_print_file (temp);
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
  uuid_t binuuid;
  uuid_generate_random(binuuid);
  char *uuid = malloc(37);
  uuid_unparse_upper(binuuid, uuid);

  new_elt->is_new_file = is_new_file;
  new_elt->is_in_modification = false;
  new_elt->next_file = NULL;
  new_elt->file = new_file;
  new_elt->uuid = uuid;

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

char *file_list_get_uuid_from_tab_view(AdwTabView *tab_view)
{
  AdwTabPage *current_page = adw_tab_view_get_selected_page (tab_view);
  GtkWidget *child = adw_tab_page_get_child (ADW_TAB_PAGE(current_page));
  GtkWidget *scrolled_bar = gtk_widget_get_first_child (GTK_WIDGET(child));
  GtkWidget *box = gtk_widget_get_first_child (GTK_WIDGET(scrolled_bar));
  GtkWidget *label = gtk_widget_get_last_child (GTK_WIDGET(box));
  g_print("TEXT : %s\n", (char *)gtk_label_get_text (GTK_LABEL(label)));

  return (char *)gtk_label_get_text (GTK_LABEL(label));
}

FileList * file_list_get_file_infos_from_tab_view(FileList *list, AdwTabView *tab_view)
{
  char *current_uuid = file_list_get_uuid_from_tab_view (tab_view);
  FileList *temp = list;

  while(temp != NULL)
    {
      if (strcmp(current_uuid, temp->uuid) == 0)
        {
          g_print("FOUND !\n");
          return temp;
        }
      temp=temp->next_file;
    }

  return NULL;
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

FileList * file_list_get_last_file_info(FileList *list)
{
  FileList *temp = list;

  if (list == NULL)
    {
      return NULL;
    }

  while(temp->next_file != NULL)
    {
      temp=temp->next_file;
    }

  return temp;
}

void file_list_print_file(FileList *file)
{
  char * file_name;

  g_print("UUID : %s\n", file->uuid);
  if (file->file != NULL) {
    file_name = g_file_get_basename (file->file);
  } else {
    file_name = "Untitled Document";
  }
  g_print("%s\n", file_name);

  if (file->is_new_file)
    {
      g_print("new file\n");
    }
  else
    {
      g_print("not new file\n");
    }
  if (file->is_in_modification)
    {
      g_print("modifications_not_saved\n");
    }
  else
    {
      g_print("no modifications not saved\n");
    }
}
