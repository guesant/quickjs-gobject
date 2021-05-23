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
#include "gi/value.hh"
#include "utils/macros.hh"

namespace QJSGir {

gsize get_type_tag_size(GITypeTag tag) {
  switch (tag) {
  case GI_TYPE_TAG_BOOLEAN:
    return sizeof(gboolean);

  case GI_TYPE_TAG_INT8:
  case GI_TYPE_TAG_UINT8:
    return sizeof(gint8);

  case GI_TYPE_TAG_INT16:
  case GI_TYPE_TAG_UINT16:
    return sizeof(gint16);

  case GI_TYPE_TAG_INT32:
  case GI_TYPE_TAG_UINT32:
    return sizeof(gint32);

  case GI_TYPE_TAG_INT64:
  case GI_TYPE_TAG_UINT64:
    return sizeof(gint64);

  case GI_TYPE_TAG_FLOAT:
    return sizeof(gfloat);

  case GI_TYPE_TAG_DOUBLE:
    return sizeof(gdouble);

  case GI_TYPE_TAG_GTYPE:
    return sizeof(GType);

  case GI_TYPE_TAG_UNICHAR:
    return sizeof(gunichar);

  case GI_TYPE_TAG_VOID:
  case GI_TYPE_TAG_UTF8:
  case GI_TYPE_TAG_FILENAME:
  case GI_TYPE_TAG_ARRAY:
  case GI_TYPE_TAG_INTERFACE:
  case GI_TYPE_TAG_GLIST:
  case GI_TYPE_TAG_GSLIST:
  case GI_TYPE_TAG_GHASH:
  case GI_TYPE_TAG_ERROR:
  default:
    ERROR("Unhandled tag type: %s", g_type_tag_to_string(tag));
  }
}

char *get_type_name(GITypeInfo *type_info) {
  GITypeTag type_tag = g_type_info_get_tag(type_info);

  switch (type_tag) {
  case GI_TYPE_TAG_BOOLEAN:
    return g_strdup("Boolean");

  case GI_TYPE_TAG_INT8:
  case GI_TYPE_TAG_INT16:
  case GI_TYPE_TAG_INT32:
  case GI_TYPE_TAG_INT64:
  case GI_TYPE_TAG_UINT8:
  case GI_TYPE_TAG_UINT16:
  case GI_TYPE_TAG_UINT32:
  case GI_TYPE_TAG_UINT64:
  case GI_TYPE_TAG_FLOAT:
  case GI_TYPE_TAG_DOUBLE:
    return g_strdup("Number");

  case GI_TYPE_TAG_GTYPE:
    return g_strdup("GType");

  case GI_TYPE_TAG_UNICHAR:
    return g_strdup("Char");

  case GI_TYPE_TAG_UTF8:
  case GI_TYPE_TAG_FILENAME:
    return g_strdup("String");

  case GI_TYPE_TAG_INTERFACE: {
    GIBaseInfo *info   = g_type_info_get_interface(type_info);
    auto        result = g_strdup_printf("%s.%s",
                                         g_base_info_get_namespace(info), g_base_info_get_name(info));
    g_base_info_unref(info);
    return result;
  }

  case GI_TYPE_TAG_ARRAY:
  case GI_TYPE_TAG_GLIST:
  case GI_TYPE_TAG_GSLIST: {
    GITypeInfo *elem_info = g_type_info_get_param_type(type_info, 0);
    char *      elem_name = get_type_name(elem_info);
    char *      result    = g_strdup_printf("%s[]", elem_name);

    g_base_info_unref(elem_info);
    g_free(elem_name);

    return result;
  }

  case GI_TYPE_TAG_GHASH:
  case GI_TYPE_TAG_ERROR:
  case GI_TYPE_TAG_VOID:
  default:
    return g_strdup(g_type_tag_to_string(type_tag));
  }
}

gsize get_type_size(GITypeInfo *type_info) {
  gsize size = 0;

  GITypeTag type_tag = g_type_info_get_tag(type_info);

  switch (type_tag) {
  case GI_TYPE_TAG_BOOLEAN:
  case GI_TYPE_TAG_INT8:
  case GI_TYPE_TAG_UINT8:
  case GI_TYPE_TAG_INT16:
  case GI_TYPE_TAG_UINT16:
  case GI_TYPE_TAG_INT32:
  case GI_TYPE_TAG_UINT32:
  case GI_TYPE_TAG_INT64:
  case GI_TYPE_TAG_UINT64:
  case GI_TYPE_TAG_FLOAT:
  case GI_TYPE_TAG_DOUBLE:
  case GI_TYPE_TAG_GTYPE:
  case GI_TYPE_TAG_UNICHAR: {
    size = get_type_tag_size(type_tag);
    break;
  }

  case GI_TYPE_TAG_INTERFACE: {
    GIBaseInfo *info;
    GIInfoType  info_type;

    info      = g_type_info_get_interface(type_info);
    info_type = g_base_info_get_type(info);

    size = sizeof(gpointer);

    switch (info_type) {
    case GI_INFO_TYPE_STRUCT:
      if (!g_type_info_is_pointer(type_info)) {
        size = g_struct_info_get_size((GIStructInfo *)info);
      }
      break;

    case GI_INFO_TYPE_UNION:
      if (!g_type_info_is_pointer(type_info)) {
        size = g_union_info_get_size((GIUnionInfo *)info);
      }
      break;

    case GI_INFO_TYPE_ENUM:
    case GI_INFO_TYPE_FLAGS:
      if (!g_type_info_is_pointer(type_info)) {
        size = get_type_tag_size(g_enum_info_get_storage_type((GIEnumInfo *)info));
      }
      break;

    case GI_INFO_TYPE_BOXED:
    case GI_INFO_TYPE_OBJECT:
    case GI_INFO_TYPE_INTERFACE:
    case GI_INFO_TYPE_CALLBACK:
      DEBUG("size for %s", g_info_type_to_string(info_type));
      break;

    case GI_INFO_TYPE_VFUNC:
    case GI_INFO_TYPE_FUNCTION:
    case GI_INFO_TYPE_CONSTANT:
    case GI_INFO_TYPE_VALUE:
    case GI_INFO_TYPE_SIGNAL:
    case GI_INFO_TYPE_PROPERTY:
    case GI_INFO_TYPE_FIELD:
    case GI_INFO_TYPE_ARG:
    case GI_INFO_TYPE_TYPE:
    case GI_INFO_TYPE_INVALID:
    case GI_INFO_TYPE_UNRESOLVED:
    default:
      ERROR("info type: %s", g_info_type_to_string(info_type));
      break;
    }

    g_base_info_unref(info);
    break;
  }

  case GI_TYPE_TAG_ARRAY:
  case GI_TYPE_TAG_VOID:
  case GI_TYPE_TAG_UTF8:
  case GI_TYPE_TAG_FILENAME:
  case GI_TYPE_TAG_GLIST:
  case GI_TYPE_TAG_GSLIST:
  case GI_TYPE_TAG_GHASH:
  case GI_TYPE_TAG_ERROR:
    size = sizeof(gpointer);
    break;
  }

  return size;
}

}
