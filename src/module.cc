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

#include <quickjs/quickjs.h>
#include "module.hh"
#include "jsapi/BootstrapGI.hh"

static int js_init(JSContext *ctx, JSModuleDef *m) {
  JS_SetModuleExport(ctx, m, "GI", QJSGir::BootstrapGI(ctx));
  return 0;
}

extern "C" {
JSModuleDef *JS_INIT_MODULE(JSContext *ctx, const char *module_name) {
  JSModuleDef *m = JS_NewCModule(ctx, module_name, js_init);

  if (!m) {
    return NULL;
  }

  JS_AddModuleExport(ctx, m, "GI");
  return m;
}
}
