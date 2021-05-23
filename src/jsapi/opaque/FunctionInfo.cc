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

#include <girffi.h>
#include <girepository.h>
#include <quickjs/quickjs.h>

#include "gi/boxed.hh"
#include "gi/value.hh"
#include "utils/error.hh"
#include "jsapi/opaque/FunctionInfo.hh"

static inline bool is_pointer_type(GITypeInfo *type_info);
static bool should_skip_return(GIBaseInfo *info, GITypeInfo *return_type);
static inline bool is_direction_out(GIDirection direction);
static inline bool is_direction_in(GIDirection direction);
static bool check_is_method(GIBaseInfo *info);

namespace QJSGir {

bool IsDestroyNotify(GIBaseInfo *info) {
  return strcmp(g_base_info_get_name(info), "DestroyNotify") == 0 &&
         strcmp(g_base_info_get_namespace(info), "GLib") == 0;
}

/**
 * The constructor just stores the GIBaseInFfo ref. The rest of the
 * initialization is done in FunctionInfo::Init, lazily.
 */
FunctionInfo::FunctionInfo(GIBaseInfo *gi_info) {
  info            = g_base_info_ref(gi_info);
  call_parameters = nullptr;
}

FunctionInfo::~FunctionInfo() {
  g_base_info_unref(info);

  if (call_parameters != nullptr) {
    g_function_invoker_destroy(&invoker);
    delete[] call_parameters;
  }
}

/**
 * Initializes the parameters metadata (number, directionality, type) and caches it
 */
bool FunctionInfo::Init(JSContext *ctx) {
  if (call_parameters != nullptr) {
    return true;
  }

  g_function_info_prep_invoker(info, &invoker, NULL);

  is_method = check_is_method(info);
  can_throw = g_callable_info_can_throw_gerror(info);

  n_callable_args = g_callable_info_get_n_args(info);
  n_total_args    = n_callable_args;
  n_out_args      = 0;
  n_in_args       = 0;

  if (is_method) {
    n_total_args++;
  }

  if (can_throw) {
    n_total_args++;
  }

  call_parameters = new Parameter[n_callable_args]();

  /*
   * Examine load parameter types and count arguments
   */

  for (int i = 0; i < n_callable_args; i++) {
    GIArgInfo  arg_info;
    GITypeInfo type_info;
    g_callable_info_load_arg((GICallableInfo *)info, i, &arg_info);
    g_arg_info_load_type(&arg_info, &type_info);

    bool        may_be_null = g_arg_info_may_be_null(&arg_info);
    GIDirection direction   = g_arg_info_get_direction(&arg_info);
    GITypeTag   tag         = g_type_info_get_tag(&type_info);

    call_parameters[i].direction = direction;

    if (call_parameters[i].type == ParameterType::SKIP) {
      continue;
    }

    // If there is an array length, this is an array
    int length_i = g_type_info_get_array_length(&type_info);
    if (tag == GI_TYPE_TAG_ARRAY && length_i >= 0) {
      call_parameters[i].type        = ParameterType::ARRAY;
      call_parameters[length_i].type = ParameterType::SKIP;

      // If array length came before, we need to remove it from args count

      if (is_direction_in(call_parameters[length_i].direction) && length_i < i) {
        n_in_args--;
      }

      if (is_direction_out(call_parameters[length_i].direction) && length_i < i) {
        n_out_args--;
      }
    } else if (tag == GI_TYPE_TAG_INTERFACE) {
      GIBaseInfo *interface_info = g_type_info_get_interface(&type_info);
      GIInfoType  interface_type = g_base_info_get_type(interface_info);

      if (interface_type == GI_INFO_TYPE_CALLBACK) {
        if (IsDestroyNotify(interface_info)) {
          /* Skip GDestroyNotify if they appear before the respective callback */
          call_parameters[i].type = ParameterType::SKIP;
        } else {
          call_parameters[i].type = ParameterType::CALLBACK;

          int destroy_i = g_arg_info_get_destroy(&arg_info);
          int closure_i = g_arg_info_get_closure(&arg_info);

          if (destroy_i >= 0 && closure_i < 0) {
            Throw::UnsupportedCallback(ctx, info);
            g_base_info_unref(interface_info);
            return false;
          }

          if (destroy_i >= 0 && destroy_i < n_callable_args) {
            call_parameters[destroy_i].type = ParameterType::SKIP;
          }

          if (closure_i >= 0 && closure_i < n_callable_args) {
            call_parameters[closure_i].type = ParameterType::SKIP;
          }

          if (destroy_i < i) {
            if (is_direction_in(call_parameters[destroy_i].direction)) {
              n_in_args--;
            }
            if (is_direction_out(call_parameters[destroy_i].direction)) {
              n_out_args--;
            }
          }

          if (closure_i < i) {
            if (is_direction_in(call_parameters[closure_i].direction)) {
              n_in_args--;
            }
            if (is_direction_out(call_parameters[closure_i].direction)) {
              n_out_args--;
            }
          }
        }
      }

      g_base_info_unref(interface_info);
    }

    if (is_direction_in(call_parameters[i].direction) && !may_be_null) {
      n_in_args++;
    }

    if (is_direction_out(call_parameters[i].direction)) {
      n_out_args++;
    }
  }

  /*
   * Examine return type
   */

  GITypeInfo return_type;
  g_callable_info_load_return_type(info, &return_type);
  int return_length_i = g_type_info_get_array_length(&return_type);

  if (return_length_i != -1) {
    n_out_args--;
  }

  if (!should_skip_return(info, &return_type)) {
    n_out_args++;
  }

  return true;
}

/**
 * Type checks the JS arguments, throwing an error.
 * @returns true if types match
 */
bool FunctionInfo::TypeCheck(
  JSContext *ctx,
  int argc,
  JSValue *argv
  ) {
  if (argc < n_in_args) {
    Throw::NotEnoughArguments(ctx, n_in_args, argc);
    return false;
  }

  /*
   * Type check every IN-argument that is not skipped
   */

  for (int in_arg = 0, i = 0; i < n_callable_args; i++) {
    Parameter&param = call_parameters[i];

    if (param.type == ParameterType::SKIP) {
      continue;
    }

    GIArgInfo arg_info;
    g_callable_info_load_arg(info, i, &arg_info);
    GIDirection direction = g_arg_info_get_direction(&arg_info);

    if (direction == GI_DIRECTION_IN || direction == GI_DIRECTION_INOUT) {
      GITypeInfo type_info;
      g_arg_info_load_type(&arg_info, &type_info);
      bool may_be_null = g_arg_info_may_be_null(&arg_info);

      if (!can_convert_jsvalue_to_giargument(ctx, &type_info, argv[in_arg], may_be_null)) {
        Throw::InvalidType(ctx, &arg_info, &type_info, argv[in_arg]);
        return false;
      }

      in_arg++;
    }
  }

  return true;
}

/**
 * Creates the JS return value from the C arguments list
 * @returns the JS return value
 */
JSValue FunctionInfo::GetReturnValue(
  JSContext *ctx,
  JSValue self,
  GITypeInfo *return_type,
  GIArgument *return_value,
  GIArgument *callable_arg_values) {
  JSValue jsReturnValue;
  int     jsReturnIndex = 0;

  if (n_out_args > 1) {
    jsReturnValue = JS_NewArray(ctx);
  }

#define ADD_RETURN(value)    if (n_out_args > 1)                                \
  JS_DefinePropertyValueUint32 (ctx, jsReturnValue, jsReturnIndex++, value, 0); \
  else                                                                          \
  jsReturnValue = (value);

  int return_length_i = g_type_info_get_array_length(return_type);

  if (!should_skip_return(info, return_type)) {
    long length = -1;

    if (return_length_i >= 0) {
      GIArgInfo length_info;
      g_callable_info_load_arg(info, return_length_i, &length_info);
      GITypeInfo length_type;
      g_arg_info_load_type(&length_info, &length_type);

      length =
        giargument_to_length(
          &length_type,
          &callable_arg_values[return_length_i],
          is_direction_out(call_parameters[return_length_i].direction));
    }

    // When a method returns the instance itself, skip the conversion and just return the
    // existent wrapper
    bool isReturningSelf = is_method && pointer_from_wrapper(self) == return_value->v_pointer;
    ADD_RETURN(isReturningSelf ? self : jsvalue_from_giargument(ctx, return_type, return_value, length))
  }

  for (int i = 0; i < n_callable_args; i++) {
    if (return_length_i == i) {
      continue;
    }

    GIArgInfo  arg_info = {};
    GITypeInfo arg_type;
    GIArgument arg_value = callable_arg_values[i];
    Parameter& param     = call_parameters[i];

    g_callable_info_load_arg(info, i, &arg_info);
    g_arg_info_load_type(&arg_info, &arg_type);

    GIDirection direction = g_arg_info_get_direction(&arg_info);

    if (direction == GI_DIRECTION_OUT || direction == GI_DIRECTION_INOUT) {
      if (param.type == ParameterType::ARRAY) {
        int       length_i = g_type_info_get_array_length(&arg_type);
        GIArgInfo length_arg;
        g_callable_info_load_arg(info, length_i, &length_arg);
        GITypeInfo length_type;
        g_arg_info_load_type(&length_arg, &length_type);
        GIDirection length_direction = g_arg_info_get_direction(&length_arg);

        param.length =
          giargument_to_length(
            &length_type,
            &callable_arg_values[length_i],
            is_direction_out(length_direction));

        JSValue result = jsvalue_from_array(ctx, &arg_type, *(void **)arg_value.v_pointer, param.length);

        ADD_RETURN(result)
      } else if (param.type == ParameterType::NORMAL) {
        if (is_pointer_type(&arg_type) && g_arg_info_is_caller_allocates(&arg_info)) {
          void *pointer = &arg_value.v_pointer;
          ADD_RETURN(jsvalue_from_giargument(ctx, &arg_type, (GIArgument *)pointer))
        } else {
          ADD_RETURN(jsvalue_from_giargument(ctx, &arg_type, (GIArgument *)arg_value.v_pointer))
        }
      }
    }
  }

#undef ADD_RETURN

  return jsReturnValue;
}

}

static inline bool is_pointer_type(GITypeInfo *type_info) {
  auto tag = g_type_info_get_tag(type_info);

  if (tag != GI_TYPE_TAG_INTERFACE) {
    return false;
  }

  auto interface_info = g_type_info_get_interface(type_info);
  auto interface_type = g_base_info_get_type(interface_info);

  bool isPointer =
    interface_type != GI_INFO_TYPE_ENUM &&
    interface_type != GI_INFO_TYPE_FLAGS;

  g_base_info_unref(interface_info);

  return isPointer;
}

static bool should_skip_return(GIBaseInfo *info, GITypeInfo *return_type) {
  return g_type_info_get_tag(return_type) == GI_TYPE_TAG_VOID ||
         g_callable_info_skip_return(info) == TRUE;
}

static inline bool is_direction_out(GIDirection direction) {
  return direction == GI_DIRECTION_OUT || direction == GI_DIRECTION_INOUT;
}

static inline bool is_direction_in(GIDirection direction) {
  return direction == GI_DIRECTION_IN || direction == GI_DIRECTION_INOUT;
}

static bool check_is_method(GIBaseInfo *info) {
  auto flags = g_function_info_get_flags(info);

  return (flags & GI_FUNCTION_IS_METHOD) != 0 &&
         (flags & GI_FUNCTION_IS_CONSTRUCTOR) == 0;
}
