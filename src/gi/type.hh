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

namespace QJSGir {

gsize get_type_tag_size(GITypeTag tag);
char *get_type_name(GITypeInfo *type_info);
gsize get_type_size(GITypeInfo *type_info);

}
