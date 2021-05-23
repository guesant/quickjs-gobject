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
#include "jsapi/opaque/FunctionInfo.hh"
#include "jsapi/opaque/JSFunctionInfo.hh"

namespace QJSGir {

static void js_function_info_finalizer(JSRuntime *rt, JSValue val) {
  FunctionInfo *func = (FunctionInfo *)JS_GetOpaque(val, js_function_info_classid);

  delete func;
}

static JSClassDef js_function_info_class = {
  "FunctionInfo",
  .finalizer = js_function_info_finalizer,
};

bool js_setup_function_info(JSContext *ctx) {
  JS_NewClassID(&js_function_info_classid);

  JSRuntime *rt = JS_GetRuntime(ctx);
  if (!JS_IsRegisteredClass(JS_GetRuntime(ctx), js_function_info_classid)) {
    JS_NewClass(rt, js_function_info_classid, &js_function_info_class);
  }

  return true;
}

JSValue JS_MakeOpaqueFunctionInfo(JSContext *ctx, FunctionInfo *func) {
  js_setup_function_info(ctx);
  JSValue opaque_func_obj = JS_NewObjectClass(ctx, js_function_info_classid);
  JS_SetOpaque(opaque_func_obj, func);
  return opaque_func_obj;
}

}
