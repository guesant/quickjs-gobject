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

class Boxed {
public:
  void *data;
  GType gtype;
  GIBaseInfo *info;
  unsigned long size;
  bool owns_memory;
  JSValue *persistent;

  static size_t GetSize(GIBaseInfo *boxed_info);
};

void *pointer_from_wrapper(JSValue value);

}
