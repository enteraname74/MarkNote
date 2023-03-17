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
  GtkTextView         *text_view;
};

G_DEFINE_FINAL_TYPE (MarknoteWindow, marknote_window, ADW_TYPE_APPLICATION_WINDOW)

static void on_open_response(GtkDialog *dialog, int response, gpointer data) {
  char *basename;
  char *contents;
  gsize length;
  GtkTextView *text_view = (GtkTextView *)data;

  if (response == GTK_RESPONSE_ACCEPT)
    {
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);

      g_autoptr(GFile) file = gtk_file_chooser_get_file (chooser);

      basename = g_file_get_basename (file);
      g_print("%s\n", basename);

      if (g_file_load_contents (file, NULL, &contents, &length, NULL, NULL)) {
        GtkTextBuffer *buffer;

        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
        gtk_text_buffer_set_text (buffer, contents, length);
        g_free(contents);
      }

      g_free(basename);
    }

  gtk_window_destroy (GTK_WINDOW (dialog));
}

static void open_file_chooser(GtkWidget *widget, gpointer data) {
  MarknoteWindow *current_window = (MarknoteWindow *)data;
  GtkWidget *dialog;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;

  dialog = gtk_file_chooser_dialog_new ("Open File",
                                        GTK_WINDOW(&current_window->parent_instance),
                                        action,
                                        "_Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        "_Open",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);

  gtk_window_present (GTK_WINDOW (dialog));

  g_signal_connect (dialog, "response",
                    G_CALLBACK (on_open_response),
                    current_window->text_view);
}

static void marknote_window_class_init (MarknoteWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/MarkNote/ui/marknote-main-window.ui");
  gtk_widget_class_bind_template_child (widget_class, MarknoteWindow, header_bar);
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, open_file);
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, text_view);

}

static void marknote_window_init (MarknoteWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (GTK_BUTTON(self->open_file), "clicked", G_CALLBACK (open_file_chooser), (gpointer)self);
}

