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

#include <girepository.h>
#include <quickjs/quickjs.h>

namespace QJSGir {

bool jsvalue_to_giargument(JSContext *ctx, GITypeInfo *type_info, GIArgument *argument, JSValue value);
bool jsvalue_to_giargument(JSContext *ctx, GITypeInfo *type_info, GIArgument *argument, JSValue value, bool may_be_null);

bool can_convert_jsvalue_to_giargument(JSContext *ctx, GITypeInfo *type_info, JSValue value, bool may_be_null);

JSValue jsvalue_from_array(JSContext *ctx, GITypeInfo *type_info, void *data, long length);
JSValue jsvalue_from_giargument(JSContext *ctx, GITypeInfo *type_info, GIArgument *argument, long length = -1, bool must_copy = false);

void free_giargument(GITypeInfo *type_info, GIArgument *arg, GITransfer transfer, GIDirection direction);
long giargument_to_length(GITypeInfo *type_info, GIArgument *arg, bool is_pointer);
void free_giargument_array(GITypeInfo *type_info, GIArgument *arg, GITransfer transfer, GIDirection direction, long length);

}
