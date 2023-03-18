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

#include "config.h"

#include "marknote-window.h"


struct FileInfoStruct {
  gboolean is_new_file;
  GFile *file;
  struct FileInfoStruct *next_file;
};

typedef struct FileInfoStruct FileList;

struct _MarknoteWindow
{
  AdwApplicationWindow  parent_instance;

  /* Template widgets */
  GtkHeaderBar        *header_bar;
  GtkButton           *open_file;
  GtkButton           *new_file;
  AdwTabView          *tab_view;
  AdwTabBar           *tab_bar;

  // Open file list :
  FileList            *file_list;
};

G_DEFINE_FINAL_TYPE (MarknoteWindow, marknote_window, ADW_TYPE_APPLICATION_WINDOW)

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

FileList * add_file_to_file_list(FileList *list, GFile *new_file, gboolean is_new_file)
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

FileList * delete_from_file_list_at(FileList *list, int pos)
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

void changed (
  GtkTextBuffer* self,
  gpointer user_data
) {
  g_print("ACTION\n");
}
static void add_tab(MarknoteWindow *window, GtkWidget *text_view, char *file_name) {

  GtkWidget *scrolled_window = gtk_scrolled_window_new ();
  GtkTextBuffer *buffer;

  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW(scrolled_window), GTK_WIDGET(text_view));
  gtk_widget_set_vexpand (GTK_WIDGET(scrolled_window), true);
  gtk_widget_set_hexpand(GTK_WIDGET(scrolled_window), true);

  AdwTabPage *tab_page = adw_tab_view_add_page (ADW_TAB_VIEW (window->tab_view), GTK_WIDGET(scrolled_window), NULL);
  adw_tab_page_set_title (ADW_TAB_PAGE(tab_page), file_name);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text_view));
  g_signal_connect (GTK_TEXT_BUFFER (buffer),"changed", G_CALLBACK (changed), NULL);
  show_file_list_infos (window->file_list);
}

static void on_open_response(GtkNativeDialog *native, int response, gpointer data)
{
  char *file_name;
  char *contents;
  gsize length;
  MarknoteWindow *window = (MarknoteWindow *)data;
  GtkWidget *text_view = gtk_text_view_new ();

  if (response == GTK_RESPONSE_ACCEPT)
    {
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (native);

      GFile * file = gtk_file_chooser_get_file (chooser);
      window->file_list = add_file_to_file_list (window->file_list, gtk_file_chooser_get_file (chooser), false);
      file_name = g_file_get_basename (file);

      if (g_file_load_contents (file, NULL, &contents, &length, NULL, NULL)) {
        GtkTextBuffer *buffer;

        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
        gtk_text_buffer_set_text (buffer, contents, length);

        add_tab (window, text_view, file_name);

        g_free(contents);
      }

      g_free(file_name);
    }

  g_object_unref(native);
}

static void save_file_complete (GObject *source_object,
                                GAsyncResult *result,
                                gpointer user_data)
{
  GFile *file = G_FILE (source_object);

  g_autoptr (GError) error = NULL;
  g_file_replace_contents_finish (file, result, NULL, &error);

  g_autofree char *display_name = NULL;
  g_autoptr (GFileInfo) info = g_file_query_info (file,
                                                  "standard::display-name",
                                                  G_FILE_QUERY_INFO_NONE,
                                                  NULL,
                                                  NULL);
  if (info != NULL)
    {
      display_name = g_strdup(g_file_info_get_attribute_string (info, "standard::display-name"));
    }
  else
    {
      display_name = g_file_get_basename (file);
    }

  if (error != NULL) {
    g_printerr("Unable to save “%s”: %s\n",
               display_name,
               error->message);
  }
}

static void save_file(MarknoteWindow *self, GFile *file)
{
  AdwTabPage *current_page = adw_tab_view_get_selected_page (ADW_TAB_VIEW(self->tab_view));
  GtkWidget *page_child = adw_tab_page_get_child (ADW_TAB_PAGE(current_page));
  GtkWidget *text_view = gtk_widget_get_first_child (GTK_WIDGET(page_child));

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text_view));

  // Retrieve the iterator at the start of the buffer
  GtkTextIter start;
  gtk_text_buffer_get_start_iter (buffer, &start);

  // Retrieve the iterator at the end of the buffer
  GtkTextIter end;
  gtk_text_buffer_get_end_iter (buffer, &end);

  // Retrieve all the visible text between the two bounds
  char *text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

  // If there is nothing to save, return early
  if (text == NULL)
   return;

  g_autoptr(GBytes) bytes = g_bytes_new_take (text, strlen (text));

  // Start the asynchronous operation to save the data into the file
  g_file_replace_contents_bytes_async (file,
                                      bytes,
                                      NULL,
                                      FALSE,
                                      G_FILE_CREATE_NONE,
                                      NULL,
                                      save_file_complete,
                                      self);
}

static void on_save_response(GtkNativeDialog *native, int response, MarknoteWindow *self)
{
  if (response == GTK_RESPONSE_ACCEPT)
    {
      g_autoptr (GFile) file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (native));

      save_file(self, file);
    }

  g_object_unref(native);
}

static void save_as_file(GAction *action G_GNUC_UNUSED,
                         GVariant *param G_GNUC_UNUSED,
                         MarknoteWindow *self)
{
  GtkFileChooserNative *native = gtk_file_chooser_native_new ("Save File As",
                                                              GTK_WINDOW(self),
                                                              GTK_FILE_CHOOSER_ACTION_SAVE,
                                                              "_Save",
                                                              "_Cancel");
  g_signal_connect (native, "response", G_CALLBACK(on_save_response), self);
  gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

static void open_file_chooser(GtkWidget *widget, gpointer data) {
  MarknoteWindow *current_window = (MarknoteWindow *)data;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;

  GtkFileChooserNative *native = gtk_file_chooser_native_new ("Open File",
                                        GTK_WINDOW(&current_window->parent_instance),
                                        action,
                                        "_Open",
                                        "_Cancel");

  g_signal_connect (native, "response",
                    G_CALLBACK (on_open_response),
                    current_window);
  gtk_native_dialog_show (GTK_NATIVE_DIALOG(native));
}

static void marknote_window_class_init (MarknoteWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/MarkNote/ui/marknote-main-window.ui");
  gtk_widget_class_bind_template_child (widget_class, MarknoteWindow, header_bar);
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, open_file);
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, new_file);
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, tab_view);
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, tab_bar);

}

static gboolean close_page(AdwTabView *view, AdwTabPage *page, gpointer user_data)
{
  MarknoteWindow *window = (MarknoteWindow *)user_data;
  g_print("Close page !");
  g_print("Pos of deleted page : %d\n",
          adw_tab_view_get_page_position (view, page)
          );
  int page_pos;
  page_pos = adw_tab_view_get_page_position (view, page);
  window->file_list = delete_from_file_list_at (window->file_list, page_pos);
  show_file_list_infos (window->file_list);
  adw_tab_view_close_page_finish (view, page, !adw_tab_page_get_pinned (page));
  return true;
}

static void create_new_file(GtkWidget *self, gpointer data)
{
  MarknoteWindow *window = (MarknoteWindow *)data;
  window->file_list = add_file_to_file_list (window->file_list, NULL, true);
  add_tab (window, gtk_text_view_new (), "Untitled document");
}

static void marknote_window_init (MarknoteWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_autoptr (GSimpleAction) save_action = g_simple_action_new ("save-as", NULL);
  g_signal_connect (save_action, "activate", G_CALLBACK (save_as_file), self);
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (save_action));

  g_signal_connect (GTK_BUTTON(self->open_file), "clicked", G_CALLBACK (open_file_chooser), (gpointer)self);
  g_signal_connect (GTK_BUTTON(self->new_file), "clicked", G_CALLBACK (create_new_file), (gpointer)self);
  g_signal_connect (ADW_TAB_VIEW(self->tab_view), "close-page", G_CALLBACK (close_page), (gpointer)self);

  adw_tab_bar_set_view (ADW_TAB_BAR(self->tab_bar), ADW_TAB_VIEW (self->tab_view));
}

