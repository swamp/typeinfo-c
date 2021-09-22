/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <memory.h>
#include <swamp-typeinfo/typeinfo.h>
#include <tiny-libc/tiny_libc.h>

void swtiInitString(SwtiStringType* self)
{
    self->internal.type = SwtiTypeString;
    self->internal.name = "String";
    self->internal.hash = 0x0000;
}

void swtiInitResourceName(SwtiResourceNameType* self)
{
    self->internal.type = SwtiTypeResourceName;
    self->internal.name = "ResourceName";
    self->internal.hash = 0x0000;
}

void swtiInitChar(SwtiCharType* self)
{
    self->internal.type = SwtiTypeChar;
    self->internal.name = "Char";
    self->internal.hash = 0x0000;
}

void swtiInitInt(SwtiIntType* self)
{
    self->internal.type = SwtiTypeInt;
    self->internal.name = "Int";
    self->internal.hash = 0x0000;
}

void swtiInitAny(SwtiAnyType* self)
{
    self->internal.type = SwtiTypeAny;
    self->internal.name = "Any";
    self->internal.hash = 0x0000;
}

void swtiInitUnmanaged(SwtiUnmanagedType* self, uint16_t userTypeId, const char* name)
{
    self->internal.type = SwtiTypeUnmanaged;
    if (name != 0) {
        self->internal.name = tc_strdup(name);
    } else {
        self->internal.name = 0;
    }
    self->internal.hash = 0x0000;
    self->userTypeId = userTypeId;
}

void swtiInitAnyMatchingTypes(SwtiAnyMatchingTypesType * self)
{
    self->internal.type = SwtiTypeAnyMatchingTypes;
    self->internal.name = "*";
    self->internal.hash = 0x0000;
}

void swtiInitFixed(SwtiFixedType* self)
{
    self->internal.type = SwtiTypeFixed;
    self->internal.name = "Fixed";
    self->internal.hash = 0x0000;
}

void swtiInitBoolean(SwtiBooleanType* self)
{
    self->internal.type = SwtiTypeBoolean;
    self->internal.name = "Bool";
    self->internal.hash = 0x0000;
}

void swtiInitBlob(SwtiBlobType* self)
{
    self->internal.type = SwtiTypeBlob;
    self->internal.name = "Blob";
    self->internal.hash = 0x0000;
}

void swtiInitArray(SwtiArrayType* self)
{
    self->internal.type = SwtiTypeArray;
    self->internal.name = "Array";
    self->internal.hash = 0x0000;
}

void swtiInitList(SwtiListType* self)
{
    self->internal.type = SwtiTypeList;
    self->internal.name = "List";
    self->internal.hash = 0x0000;
    self->internal.index = 0xffff;
}

void swtiInitFunction(SwtiFunctionType* self, const SwtiType** types, size_t typeCount)
{
    self->internal.type = SwtiTypeFunction;
    self->internal.name = "Function";
    self->internal.hash = 0x0000;
    self->parameterCount = typeCount;
    self->parameterTypes = calloc(typeCount, sizeof(SwtiType*));
    tc_memcpy_type_n(self->parameterTypes, types, typeCount);
}

void swtiInitTuple(SwtiTupleType* self, const SwtiType** types, size_t typeCount)
{
    self->internal.type = SwtiTypeTuple;
    self->internal.name = "Tuple";
    self->internal.hash = 0x0000;
    self->fieldCount = typeCount;
    self->fields = calloc(typeCount, sizeof(SwtiTupleTypeField *));
    self->types = calloc(typeCount, sizeof(SwtiType *));
    if (types != 0) {
        tc_memcpy_octets(self->types, types, sizeof(SwtiType*) * typeCount);
    }
}


void swtiInitRecord(SwtiRecordType* self)
{
    self->internal.type = SwtiTypeRecord;
    self->internal.name = "Record";
    self->internal.hash = 0x0000;
    self->fieldCount = 0;
    self->fields = 0;
}

void swtiInitRecordWithFields(SwtiRecordType* self, const SwtiRecordTypeField fields[], size_t fieldCount)
{
    swtiInitRecord(self);
    self->fieldCount = fieldCount;
    self->fields = calloc(fieldCount, sizeof(SwtiRecordTypeField));
    tc_memcpy_type_n(self->fields, fields, fieldCount);
}

void swtiInitCustom(SwtiCustomType* self, const char* name, const SwtiCustomTypeVariant variants[], size_t variantCount)
{
    self->internal.type = SwtiTypeCustom;
    self->internal.name = name;
    self->internal.hash = 0x0000;
    self->variantCount = variantCount;
    self->variantTypes = calloc(variantCount, sizeof(SwtiCustomTypeVariant));
    self->generic.genericTypes = 0;
    self->generic.genericCount = 0;
    tc_memcpy_type_n(self->variantTypes, variants, variantCount);
}

static void initGenerics(SwtiGenericParams* self, const SwtiType* types[], size_t typeCount)
{
    self->genericCount = typeCount;
    self->genericTypes = calloc(typeCount, sizeof(SwtiType*));
    tc_memcpy_type_n(self->genericTypes, types, typeCount);
}

void swtiInitCustomWithGenerics(SwtiCustomType* self, const char* name,  const SwtiType* types[], size_t typeCount,
                                const SwtiCustomTypeVariant variants[], size_t variantCount)
{
    swtiInitCustom(self, name, variants, variantCount);
    initGenerics(&self->generic, types, typeCount);
}

void swtiInitAlias(SwtiAliasType* self, const char* name, const SwtiType* targetType)
{
    self->internal.type = SwtiTypeAlias;
    self->internal.name = name;
    self->internal.hash = 0x0000;
    self->targetType = targetType;
}
