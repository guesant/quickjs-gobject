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

#include <glib.h>
#include <girepository.h>
#include <quickjs/quickjs.h>

#include "gi/type.hh"

namespace QJSGir::Throw {

void NotEnoughArguments(JSContext *ctx, int expected, int actual) {
  char *msg = g_strdup_printf(
    "Not enough arguments; expected %i, have %i",
    expected, actual);

  JS_ThrowTypeError(ctx, "%s", msg);
  g_free(msg);
}

void UnsupportedCallback(JSContext *ctx, GIBaseInfo *info) {
  char *message = g_strdup_printf("Function %s.%s has a GDestroyNotify but no user_data, not supported",
                                  g_base_info_get_namespace(info),
                                  g_base_info_get_name(info));

  JS_ThrowTypeError(ctx, "%s", message);
  g_free(message);
}

void InvalidType(JSContext *ctx, GIArgInfo *info, GITypeInfo *type_info, JSValue value) {
  char *expected = get_type_name(type_info);
  char *msg      = g_strdup_printf(
    "Expected argument of type %s for parameter %s, got '%s'",
    expected,
    g_base_info_get_name(info),
    JS_ToCString(ctx, value));

  JS_ThrowTypeError(ctx, "%s", msg);
  g_free(expected);
  g_free(msg);
}

}
