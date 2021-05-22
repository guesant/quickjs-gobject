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

#include <girepository.h>
#include <quickjs/quickjs.h>
#include "gi/function.hh"
#include "jsapi/BootstrapGI.hh"

namespace QJSGir {

static void DefineFunction(JSContext *ctx, JSValue module_obj, GIBaseInfo *info) {
  const char *function_name = g_base_info_get_name((GIBaseInfo *)info);

  JSValue fn = QJSGir::MakeFunction(info);

  JS_DefinePropertyValueStr(ctx, module_obj, function_name, fn, 0);
}

static void DefineFunction(JSContext *ctx, JSValue module_obj, GIBaseInfo *info, const char *base_name) {
  JSValue fn = QJSGir::MakeFunction(info);

  char *function_name = g_strdup_printf("%s_%s", base_name, g_base_info_get_name(info));

  JS_DefinePropertyValueStr(ctx, module_obj, function_name, fn, 0);
  g_free(function_name);
}

static void DefineObjectFunctions(JSContext *ctx, JSValue module_obj, GIBaseInfo *info) {
  const char *object_name = g_base_info_get_name((GIBaseInfo *)info);

  int n_methods = g_object_info_get_n_methods(info);

  for (int i = 0; i < n_methods; i++) {
    GIFunctionInfo *meth_info = g_object_info_get_method(info, i);
    DefineFunction(ctx, module_obj, meth_info, object_name);
    g_base_info_unref((GIBaseInfo *)meth_info);
  }
}

static void DefineBoxedFunctions(JSContext *ctx, JSValue module_obj, GIBaseInfo *info) {
  const char *object_name = g_base_info_get_name((GIBaseInfo *)info);

  int n_methods = g_struct_info_get_n_methods(info);

  for (int i = 0; i < n_methods; i++) {
    GIFunctionInfo *meth_info = g_struct_info_get_method(info, i);
    DefineFunction(ctx, module_obj, meth_info, object_name);
    g_base_info_unref((GIBaseInfo *)meth_info);
  }
}

static void DefineBootstrapInfo(JSContext *ctx, JSValue module_obj, GIBaseInfo *info) {
  GIInfoType type = g_base_info_get_type(info);

  switch (type) {
  case GI_INFO_TYPE_FUNCTION:
    DefineFunction(ctx, module_obj, info);
    break;

  case GI_INFO_TYPE_OBJECT:
    DefineObjectFunctions(ctx, module_obj, info);
    break;

  case GI_INFO_TYPE_BOXED:
  case GI_INFO_TYPE_STRUCT:
    DefineBoxedFunctions(ctx, module_obj, info);
    break;

  default:
    break;
  }
}

JSValue BootstrapGI(JSContext *ctx) {
  GIRepository *repo  = g_irepository_get_default();
  GError *      error = NULL;

  const char *ns = "GIRepository";

  g_irepository_require(repo, ns, NULL, (GIRepositoryLoadFlags)0, &error);

  if (error) {
    return JS_Throw(ctx, JS_NewString(ctx, error->message));
  }

  JSValue module_obj = JS_NewObject(ctx);

  int n = g_irepository_get_n_infos(repo, ns);
  for (int i = 0; i < n; i++) {
    GIBaseInfo *info = g_irepository_get_info(repo, ns, i);
    DefineBootstrapInfo(ctx, module_obj, info);
  }

  return module_obj;
}

}
