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

  //adw_tab_view_append(ADW_TAB_VIEW (window->tab_view), GTK_WIDGET(scrolled_window));
  AdwTabPage *tab_page = adw_tab_view_add_page (ADW_TAB_VIEW (window->tab_view), GTK_WIDGET(scrolled_window), NULL);
  adw_tab_page_set_title (ADW_TAB_PAGE(tab_page), file_name);
}

static void on_open_response(GtkDialog *dialog, int response, gpointer data) {
  char *file_name;
  char *contents;
  gsize length;
  MarknoteWindow *window = (MarknoteWindow *)data;
  GtkWidget *text_view = gtk_text_view_new ();

  if (response == GTK_RESPONSE_ACCEPT)
    {
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);

      g_autoptr(GFile) file = gtk_file_chooser_get_file (chooser);

      file_name = g_file_get_basename (file);
      g_print("%s\n", file_name);

      if (g_file_load_contents (file, NULL, &contents, &length, NULL, NULL)) {
        GtkTextBuffer *buffer;

        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
        gtk_text_buffer_set_text (buffer, contents, length);

        add_tab (window, text_view, file_name);

        g_free(contents);
      }

      g_free(file_name);
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
                    current_window);
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

  g_signal_connect (GTK_BUTTON(self->open_file), "clicked", G_CALLBACK (open_file_chooser), (gpointer)self);

  adw_tab_bar_set_view (ADW_TAB_BAR(self->tab_bar), ADW_TAB_VIEW (self->tab_view));
}

