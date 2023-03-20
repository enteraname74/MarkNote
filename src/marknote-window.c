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
#include "marknote-file-list.h"

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
  gboolean            are_file_shortcuts_enabled;
};

G_DEFINE_FINAL_TYPE (MarknoteWindow, marknote_window, ADW_TYPE_APPLICATION_WINDOW)

void changed (
  GtkTextBuffer* self,
  gpointer user_data
) {
  g_print("ACTION\n");
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
      GFile * file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (native));

      // Search the current page shown :

      AdwTabPage *current_page = adw_tab_view_get_selected_page(ADW_TAB_VIEW(self->tab_view));
      adw_tab_page_set_title (ADW_TAB_PAGE(current_page), g_file_get_basename (file));

      int pos = adw_tab_view_get_page_position (ADW_TAB_VIEW(self->tab_view), ADW_TAB_PAGE(current_page));

      // Update file information :
      FileList *file_info = file_list_get_file_info_from_pos (self->file_list, pos);
      file_info->file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (native));
      file_info->is_new_file = false;

      save_file(self, file);
    }

  g_object_unref(native);
}

static void save_as_file(GAction *action G_GNUC_UNUSED,
                         GVariant *param G_GNUC_UNUSED,
                         MarknoteWindow *self)
{
  GtkFileChooser *chooser;
  FileList * current_file_info;
  GtkFileChooserNative *native = gtk_file_chooser_native_new ("Save File As",
                                                              GTK_WINDOW(self),
                                                              GTK_FILE_CHOOSER_ACTION_SAVE,
                                                              "_Save",
                                                              "_Cancel");

  // Specify name of current file if it isn't a new one :
  chooser = GTK_FILE_CHOOSER(native);
  current_file_info = file_list_get_file_infos_from_tab_view (self->file_list, self->tab_view);
  if (current_file_info->is_new_file)
    {
      gtk_file_chooser_set_current_name (chooser, "Untitled document");
    }
  else
    {
      g_print("%s\n",g_file_get_path (current_file_info->file));
      g_print("%s\n",g_file_get_uri (current_file_info->file));
      gtk_file_chooser_set_file(chooser, current_file_info->file, NULL);
    }

  g_signal_connect (native, "response", G_CALLBACK(on_save_response), self);
  gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

static void try_rapid_save(GAction *action  G_GNUC_UNUSED,
                           GVariant *param  G_GNUC_UNUSED,
                           MarknoteWindow  *self)
{
  // Search the current page shown :
  AdwTabPage *current_page = adw_tab_view_get_selected_page(ADW_TAB_VIEW(self->tab_view));
  int pos = adw_tab_view_get_page_position (ADW_TAB_VIEW(self->tab_view), ADW_TAB_PAGE(current_page));

  FileList *file_info = file_list_get_file_info_from_pos (self->file_list, pos);

  if (file_info->is_new_file)
    {
      save_as_file (NULL, NULL, self);
    }
  else
    {
      save_file (self, file_info->file);
    }
}

static void add_tab(MarknoteWindow *window, GtkWidget *text_view, char *file_name)
{

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

  // Enable shortcuts if previously disabled :
  if (window->are_file_shortcuts_enabled == false)
    {
      // Save action :
      g_autoptr (GSimpleAction) save_action = g_simple_action_new ("save", NULL);
      g_signal_connect (save_action, "activate", G_CALLBACK (try_rapid_save), window);
      g_action_map_add_action (G_ACTION_MAP (window), G_ACTION (save_action));

      // Save as action :
      g_autoptr (GSimpleAction) save_as_action = g_simple_action_new ("save-as", NULL);
      g_signal_connect (save_as_action, "activate", G_CALLBACK (save_as_file), window);
      g_action_map_add_action (G_ACTION_MAP (window), G_ACTION (save_as_action));

      window->are_file_shortcuts_enabled = true;
    }

  // We change the current selected page to the new one :
  adw_tab_view_set_selected_page (window->tab_view, tab_page);
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

      // If opened file is already in our list of files, we simply put focus on the file :
      if (file_list_search_file (window->file_list, file))
        {
          g_print("File in list !\n");
          int page_position = file_list_get_pos_of_file_from_path (window->file_list, g_file_get_path (file));
          if (page_position != -1)
            {
              g_print("Page position : %d\n", page_position);
              AdwTabPage *new_selected_page = adw_tab_view_get_nth_page (window->tab_view, page_position);
              adw_tab_view_set_selected_page (window->tab_view, new_selected_page);
              return;
            }
        }

      window->file_list = file_list_add_file (window->file_list, gtk_file_chooser_get_file (chooser), false);
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

static void create_new_file(GtkWidget *self, gpointer data)
{
  MarknoteWindow *window = (MarknoteWindow *)data;
  window->file_list = file_list_add_file (window->file_list, NULL, true);
  add_tab (window, gtk_text_view_new (), "Untitled document");
}

static void page_reordered (AdwTabView* self,
                            AdwTabPage* page,
                            gint position,
                            gpointer user_data)
{
  g_print("New position for page : %d\n", position);
}

static gboolean tab_view_close_page(AdwTabView *view, AdwTabPage *page, gpointer user_data)
{
  MarknoteWindow *window = (MarknoteWindow *)user_data;
  g_print("Close page !");
  g_print("Pos of deleted page : %d\n",
          adw_tab_view_get_page_position (view, page)
          );
  int page_pos;
  page_pos = adw_tab_view_get_page_position (view, page);
  window->file_list = file_list_delete_at (window->file_list, page_pos);
  show_file_list_infos (window->file_list);
  adw_tab_view_close_page_finish (view, page, !adw_tab_page_get_pinned (page));

  // Disable shortcuts and save options if no files are left open :
  if (file_list_get_length (window->file_list) == 1)
    {
      g_action_map_remove_action (G_ACTION_MAP (window), "save-as");
      window->are_file_shortcuts_enabled = false;
    }

  return true;
}

static void marknote_window_init (MarknoteWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  // Initialize shortcuts to be disabled (no files are open at the beginning) :
  self->are_file_shortcuts_enabled = false;

  g_signal_connect (GTK_BUTTON(self->open_file), "clicked", G_CALLBACK (open_file_chooser), (gpointer)self);
  g_signal_connect (GTK_BUTTON(self->new_file), "clicked", G_CALLBACK (create_new_file), (gpointer)self);
  g_signal_connect (ADW_TAB_VIEW(self->tab_view), "close-page", G_CALLBACK (tab_view_close_page), (gpointer)self);
  g_signal_connect (ADW_TAB_VIEW(self->tab_view), "page-reordered", G_CALLBACK (page_reordered), (gpointer)self);

  adw_tab_bar_set_view (ADW_TAB_BAR(self->tab_bar), ADW_TAB_VIEW (self->tab_view));
}
