/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <imprint/allocator.h>
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

void swtiInitUnmanaged(SwtiUnmanagedType* self, uint16_t userTypeId, const char* name, ImprintAllocator* allocator)
{
    self->internal.type = SwtiTypeUnmanaged;
    if (name != 0) {
        self->internal.name = imprintStrDup(allocator, name);
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

void swtiInitFunction(SwtiFunctionType* self, const SwtiType** types, size_t typeCount, ImprintAllocator* allocator)
{
    self->internal.type = SwtiTypeFunction;
    self->internal.name = "Function";
    self->internal.hash = 0x0000;
    self->parameterCount = typeCount;
    self->parameterTypes = IMPRINT_CALLOC_TYPE_COUNT(allocator, const SwtiType*, typeCount);
    tc_memcpy_type_n(self->parameterTypes, types, typeCount);
}

int swtiVerifyMemoryInfo(const SwtiMemoryInfo* info)
{
    if (info->memoryAlign < 1 || info->memoryAlign > 8) {
        return -55;
    }

    if (info->memorySize < 1 || info->memorySize > 256) {
        return -68;
    }

    return 0;
}

int swtiVerifyMemoryOffsetInfo(const SwtiMemoryOffsetInfo* info)
{
    if (swtiVerifyMemoryInfo(&info->memoryInfo) < 0) {
        return -54;
    }

    if (info->memoryOffset > 256) {
        return -52;
    }

    return 0;
}

int swtiVerifyTuple(const SwtiTupleType* self)
{
    if (self->fieldCount < 1 || self->fieldCount > 16) {
        return -1;
    }

    for (size_t i=0; i<self->fieldCount; ++i) {
        const SwtiTupleTypeField* field = &self->fields[i];
        if (!field->name || *field->name == 0xbe) {
            return -4;
        }
        if (swtiVerifyMemoryOffsetInfo(&field->memoryOffsetInfo) < 0) {
            return -5;
        }
    }

    return 0;
}


void swtiInitTuple(SwtiTupleType* self, const SwtiTupleTypeField sourceFields[], size_t typeCount, ImprintAllocator* allocator)
{
    self->internal.type = SwtiTypeTuple;
    self->internal.name = "Tuple";
    self->internal.hash = 0x0000;
    self->fieldCount = typeCount;

    self->fields = IMPRINT_CALLOC_TYPE_COUNT(allocator, SwtiTupleTypeField, typeCount);
    for (size_t i=0; i<self->fieldCount; ++i) {
        ((SwtiTupleTypeField *)&self->fields[i])->memoryOffsetInfo = sourceFields[i].memoryOffsetInfo;
        *(char **)&self->fields[i].name = tc_str_dup(sourceFields[i].name);
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

void swtiInitRecordWithFields(SwtiRecordType* self, const SwtiRecordTypeField fields[], size_t fieldCount, ImprintAllocator* allocator)
{
    swtiInitRecord(self);
    self->fieldCount = fieldCount;
    self->fields = IMPRINT_CALLOC_TYPE_COUNT(allocator, SwtiRecordTypeField, fieldCount);
    tc_memcpy_type_n(self->fields, fields, fieldCount);
}

void swtiInitCustom(SwtiCustomType* self, const char* name, const SwtiCustomTypeVariant variants[], size_t variantCount, ImprintAllocator* allocator)
{
    self->internal.type = SwtiTypeCustom;
    self->internal.name = name;
    self->internal.hash = 0x0000;
    self->variantCount = variantCount;
    self->variantTypes = IMPRINT_CALLOC_TYPE_COUNT(allocator, SwtiCustomTypeVariant, variantCount);
    self->generic.genericTypes = 0;
    self->generic.genericCount = 0;
    tc_memcpy_type_n(self->variantTypes, variants, variantCount);
}

static void initGenerics(SwtiGenericParams* self, const SwtiType* types[], size_t typeCount, ImprintAllocator* allocator)
{
    self->genericCount = typeCount;
    self->genericTypes = IMPRINT_CALLOC_TYPE_COUNT(allocator, const SwtiType*, typeCount);
    tc_memcpy_type_n(self->genericTypes, types, typeCount);
}

void swtiInitCustomWithGenerics(SwtiCustomType* self, const char* name,  const SwtiType* types[], size_t typeCount,
                                const SwtiCustomTypeVariant variants[], size_t variantCount, struct ImprintAllocator* allocator)
{
    swtiInitCustom(self, name, variants, variantCount, allocator);
    initGenerics(&self->generic, types, typeCount, allocator);
}

void swtiInitAlias(SwtiAliasType* self, const char* name, const SwtiType* targetType)
{
    self->internal.type = SwtiTypeAlias;
    self->internal.name = name;
    self->internal.hash = 0x0000;
    self->targetType = targetType;
}
