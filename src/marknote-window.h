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

extern const char DEFAULT_FILE_NAME[];

static void save_file_complete (GObject *source_object,
                                GAsyncResult *result,
                                gpointer user_data);
static void save_file(MarknoteWindow *self, GFile *file);
static void on_save_response(GtkNativeDialog *native, int response, MarknoteWindow *self);
static gboolean tab_view_close_page(AdwTabView *view, AdwTabPage *page, gpointer user_data);
static void save_as_file(GAction *action G_GNUC_UNUSED,
                         GVariant *param G_GNUC_UNUSED,
                         MarknoteWindow *self);
static void try_rapid_save(GAction *action  G_GNUC_UNUSED,
                           GVariant *param  G_GNUC_UNUSED,
                           MarknoteWindow  *self);
static void add_tab(MarknoteWindow *window, GtkWidget *text_view, char *file_name);
static void on_open_response(GtkNativeDialog *native, int response, gpointer data);
static void open_file_chooser(GtkWidget *widget, gpointer data);
static void create_new_file(GtkWidget *self, gpointer data);
static void header_bar_change_title_widget(GtkHeaderBar *header_bar, char *new_title, gboolean file_in_modification);
GtkTextView * get_text_view_from_tab_view(AdwTabView *tab_view);
G_END_DECLS
