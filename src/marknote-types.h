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

G_BEGIN_DECLS

#define MARKNOTE_TYPE_APPLICATION (marknote_application_get_type())
G_DECLARE_FINAL_TYPE (MarknoteApplication, marknote_application, MARKNOTE, APPLICATION, AdwApplication)

#define MARKNOTE_TYPE_WINDOW (marknote_window_get_type())
G_DECLARE_FINAL_TYPE (MarknoteWindow, marknote_window, MARKNOTE, WINDOW, AdwApplicationWindow)

G_END_DECLS
