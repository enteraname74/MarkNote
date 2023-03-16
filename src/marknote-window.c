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
  GtkStack            *stack;
  GtkStackPage        *page1;
  GtkStackPage        *page2;
  GtkButton           *button_page1;
  GtkButton           *back_button;
};

G_DEFINE_FINAL_TYPE (MarknoteWindow, marknote_window, ADW_TYPE_APPLICATION_WINDOW)

static void go_to_editor_mode(GtkWidget *widget, gpointer data){
  MarknoteWindow *current_window = (MarknoteWindow *)data;

  gtk_stack_set_transition_type (GTK_STACK(current_window->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
  gtk_stack_set_visible_child_name (GTK_STACK(current_window->stack), "page2");
}

static void go_back_to_main_menu(GtkWidget *widget, gpointer data){
  MarknoteWindow *current_window = (MarknoteWindow *)data;

  gtk_stack_set_transition_type (GTK_STACK(current_window->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);
  gtk_stack_set_visible_child_name (GTK_STACK(current_window->stack), "page1");
}

static void marknote_window_class_init (MarknoteWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/MarkNote/ui/marknote-main-window.ui");
  gtk_widget_class_bind_template_child (widget_class, MarknoteWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, MarknoteWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, MarknoteWindow, page1);
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, page2);
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, button_page1);
  gtk_widget_class_bind_template_child(widget_class, MarknoteWindow, back_button);
}

static void marknote_window_init (MarknoteWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_stack_set_transition_duration (GTK_STACK(self->stack) , 500);
  g_signal_connect (GTK_BUTTON(self->button_page1), "clicked", G_CALLBACK (go_to_editor_mode), (gpointer)self);
  g_signal_connect (GTK_BUTTON(self->back_button), "clicked", G_CALLBACK (go_back_to_main_menu), (gpointer)self);
}

