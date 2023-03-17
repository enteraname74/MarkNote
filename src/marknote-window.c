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

struct _MarknoteWindow
{
  AdwApplicationWindow  parent_instance;

  /* Template widgets */
  GtkHeaderBar        *header_bar;
  GtkButton           *open_file;
  AdwTabView          *tab_view;
  AdwTabBar           *tab_bar;
};

G_DEFINE_FINAL_TYPE (MarknoteWindow, marknote_window, ADW_TYPE_APPLICATION_WINDOW)

static void add_tab(MarknoteWindow *window, GtkWidget *text_view, char *file_name) {

  GtkWidget *scrolled_window = gtk_scrolled_window_new ();

  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW(scrolled_window), GTK_WIDGET(text_view));
  gtk_widget_set_vexpand (GTK_WIDGET(scrolled_window), true);
  gtk_widget_set_hexpand(GTK_WIDGET(scrolled_window), true);

  AdwTabPage *tab_page = adw_tab_view_add_page (ADW_TAB_VIEW (window->tab_view), GTK_WIDGET(scrolled_window), NULL);
  adw_tab_page_set_title (ADW_TAB_PAGE(tab_page), file_name);
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

      g_autoptr(GFile) file = gtk_file_chooser_get_file (chooser);

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
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, tab_view);
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, tab_bar);

}

static void marknote_window_init (MarknoteWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_autoptr (GSimpleAction) save_action = g_simple_action_new ("save-as", NULL);
  g_signal_connect (save_action, "activate", G_CALLBACK (save_as_file), self);
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (save_action));


  g_signal_connect (GTK_BUTTON(self->open_file), "clicked", G_CALLBACK (open_file_chooser), (gpointer)self);

  adw_tab_bar_set_view (ADW_TAB_BAR(self->tab_bar), ADW_TAB_VIEW (self->tab_view));
}

