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

#include <girffi.h>
#include <girepository.h>
#include <quickjs/quickjs.h>

namespace QJSGir {

enum ParameterType {
  NORMAL, ARRAY, SKIP, CALLBACK
};

struct Parameter {
  ParameterType type;
  GIDirection   direction;
  GIArgument    data;
  long          length;
};

struct FunctionInfo {
  GIFunctionInfo *  info;
  GIFunctionInvoker invoker;

  bool              is_method;
  bool              can_throw;

  int               n_callable_args;
  int               n_total_args;
  int               n_out_args;
  int               n_in_args;

  Parameter *       call_parameters;

  FunctionInfo(GIBaseInfo *info);
  ~FunctionInfo();

  bool Init(JSContext *ctx);

  bool TypeCheck(JSContext *ctx, int argc, JSValue *argv);
  JSValue GetReturnValue(JSContext *ctx, JSValue self, GITypeInfo *return_type, GIArgument *returnvalue, GIArgument *callable_arg_values);
  void FreeReturnValue(GIArgument *info);
};

}
