/**
 * This file is part of quickjs-gobject.
 *
 * quickjs-gobject is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * quickjs-gobject is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with quickjs-gobject. If not, see <https://www.gnu.org/licenses/>
 **/

#pragma once

#include <stdio.h>

#define countof(x)    (sizeof(x) / sizeof((x)[0]))

#ifdef NDEBUG
#define DEBUG(...)    do {} while (0)
#else
#define DEBUG(...)                      \
  do {                                  \
    printf("\x1b[1;38;5;226m[DEBUG] "); \
    printf(__VA_ARGS__);                \
    printf("\n");                       \
  } while (0)
#endif

#define WARN(...)                      \
  do {                                 \
    printf("\x1b[1;38;5;202m[WARN] "); \
    printf(__VA_ARGS__);               \
    printf("\n");                      \
  } while (0)

#define ERROR(...)              \
  do {                          \
    printf("\x1b[91m[ERROR] "); \
    printf(__VA_ARGS__);        \
    printf("\n");               \
    g_assert_not_reached();     \
  } while (0)
