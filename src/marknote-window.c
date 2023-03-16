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
#include "marknote-editor-window.h"

struct _MarknoteWindow
{
  AdwApplicationWindow  parent_instance;

  /* Template widgets */
  GtkHeaderBar        *header_bar;
  GtkLabel            *label;
  GtkButton           *new_windows_button;
};

G_DEFINE_FINAL_TYPE (MarknoteWindow, marknote_window, ADW_TYPE_APPLICATION_WINDOW)

static void click_button(GtkWidget *widget, gpointer data){
  MarknoteWindow *current_window = (MarknoteWindow *)data;
  GtkWindow *window;

  window = g_object_new (MARKNOTE_TYPE_EDITOR_WINDOW,
                           "application", current_window->parent_instance,
                           NULL);

  gtk_window_present (window);
}

static void marknote_window_class_init (MarknoteWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/MarkNote/ui/marknote-main-window.ui");
  gtk_widget_class_bind_template_child (widget_class, MarknoteWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, MarknoteWindow, label);
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, new_windows_button);
}

static void marknote_window_init (MarknoteWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (GTK_BUTTON(self->new_windows_button), "clicked", G_CALLBACK (click_button), (gpointer)self);
}
