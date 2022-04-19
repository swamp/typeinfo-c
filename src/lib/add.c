/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <imprint/allocator.h>
#include <swamp-typeinfo/add.h>
#include <swamp-typeinfo/chunk.h>
#include <swamp-typeinfo/typeinfo.h>

static int addType(SwtiChunk* target, const SwtiType* source, const SwtiType** out, ImprintAllocator* allocator);

static int addTypes(SwtiChunk* target, const SwtiType** source, const SwtiType*** out, size_t count, ImprintAllocator* allocator)
{
    int error;
    *out = 0;
    const SwtiType** targetArray = IMPRINT_CALLOC_TYPE_COUNT(allocator, const SwtiType*, count);

    for (size_t i = 0; i < count; ++i) {
        if ((error = addType(target, source[i], &targetArray[i], allocator)) < 0) {
            return error;
        }
    }

    *out = targetArray;

    return 0;
}

static int addCustomTypeVariant(SwtiChunk* target, const SwtiCustomTypeVariant* source, SwtiCustomTypeVariant* out, ImprintAllocator* allocator)
{
    out->paramCount = source->paramCount;
    out->name = tc_str_dup(source->name);
    out->fields = IMPRINT_CALLOC_TYPE_COUNT(allocator, SwtiCustomTypeVariantField, out->paramCount);

    for (size_t i=0; i<out->paramCount; ++i) {
        addType(target, source->fields[i].fieldType, (const SwtiType**) &out->fields[i].fieldType, allocator);
        ((SwtiCustomTypeVariantField*)&out->fields[i])->memoryOffsetInfo.memoryOffset = source->fields[i].memoryOffsetInfo.memoryOffset;
    }

    return 0;
}

static int addCustomType(SwtiChunk* target, const SwtiCustomType* source, const SwtiCustomType** out, ImprintAllocator* allocator)
{
    SwtiCustomType* custom = IMPRINT_ALLOC_TYPE(allocator, SwtiCustomType);
    swtiInitCustom(custom, tc_str_dup(source->internal.name), 0, 0, allocator);

    custom->variantTypes = IMPRINT_CALLOC_TYPE_COUNT(allocator, SwtiCustomTypeVariant, source->variantCount);
    custom->variantCount = source->variantCount;


    *out = custom;

    int error;
    for (size_t i = 0; i < source->variantCount; ++i) {
        if ((error = addCustomTypeVariant(target, &source->variantTypes[i],
                                          (SwtiCustomTypeVariant*) &custom->variantTypes[i], allocator)) != 0) {
            return error;
        }
    }

    return 0;
}

static int addUnmanaged(SwtiChunk* target, const SwtiUnmanagedType* source, const SwtiUnmanagedType** out, ImprintAllocator* allocator)
{
    SwtiUnmanagedType* unmanagedType = IMPRINT_ALLOC_TYPE(allocator, SwtiUnmanagedType);
    swtiInitUnmanaged(unmanagedType, source->userTypeId, source->internal.name, allocator);

    *out = unmanagedType;

    return 0;
}

static int addFunction(SwtiChunk* target, const SwtiFunctionType* source, const SwtiFunctionType** out, ImprintAllocator* allocator)
{
    SwtiFunctionType* fn = IMPRINT_ALLOC_TYPE(allocator, SwtiFunctionType);
    swtiInitFunction(fn, source->parameterTypes, source->parameterCount, allocator);
    *out = fn;

    return addTypes(target, source->parameterTypes, &fn->parameterTypes, source->parameterCount, allocator);
}

static int addTupleField(SwtiChunk* target, const SwtiTupleTypeField* source, SwtiTupleTypeField* out, ImprintAllocator* allocator)
{
    if (!source->name) {
        CLOG_ERROR("name must be set")
    }
    out->name = tc_str_dup(source->name);
    out->memoryOffsetInfo = source->memoryOffsetInfo;
    return addType(target, source->fieldType, &out->fieldType, allocator);
}

static int addTuple(SwtiChunk* target, const SwtiTupleType* source, const SwtiTupleType** out, ImprintAllocator* allocator)
{
    SwtiTupleType* tuple = IMPRINT_ALLOC_TYPE(allocator, SwtiTupleType);
    tuple->fields = IMPRINT_CALLOC_TYPE_COUNT(allocator, SwtiTupleTypeField, source->fieldCount);
    tuple->fieldCount = source->fieldCount;
    tuple->memoryInfo = source->memoryInfo;

    int error;
    for (size_t i = 0; i < source->fieldCount; ++i) {
        if ((error = addTupleField(target, &source->fields[i], (struct SwtiTupleTypeField*) &tuple->fields[i], allocator)) < 0) {
            *out = 0;
            return error;
        }
    }

    *out = tuple;
    return 0;
}

static int addAlias(SwtiChunk* target, const SwtiAliasType* source, const SwtiAliasType** out, ImprintAllocator* allocator)
{
    SwtiAliasType* alias = IMPRINT_ALLOC_TYPE(allocator, SwtiAliasType);
    swtiInitAlias(alias, source->internal.name, 0);
    *out = alias;
    return addType(target, source->targetType, &alias->targetType, allocator);
}

static int addRecordField(SwtiChunk* target, const SwtiRecordTypeField* source, SwtiRecordTypeField* out, ImprintAllocator* allocator)
{
    out->name = tc_str_dup(source->name);
    out->memoryOffsetInfo = source->memoryOffsetInfo;
    return addType(target, source->fieldType, &out->fieldType, allocator);
}

static int addRecord(SwtiChunk* target, const SwtiRecordType* source, const SwtiRecordType** out, ImprintAllocator* allocator)
{
    SwtiRecordType* record = IMPRINT_ALLOC_TYPE(allocator, SwtiRecordType);
    swtiInitRecord(record);
    record->fields = IMPRINT_CALLOC_TYPE_COUNT(allocator, SwtiRecordTypeField, source->fieldCount);
    record->fieldCount = source->fieldCount;

    int error;
    for (size_t i = 0; i < source->fieldCount; ++i) {
        if ((error = addRecordField(target, &source->fields[i], (struct SwtiRecordTypeField*) &record->fields[i], allocator)) < 0) {
            *out = 0;
            return error;
        }
    }

    *out = record;
    return 0;
}

static int addArray(SwtiChunk* target, const SwtiArrayType* source, const SwtiArrayType** out, ImprintAllocator* allocator)
{
    SwtiArrayType* array = IMPRINT_ALLOC_TYPE(allocator, SwtiArrayType);
    swtiInitArray(array);
    *out = array;
    array->memoryInfo = source->memoryInfo;

    return addType(target, source->itemType, &array->itemType, allocator);
}

static int addList(SwtiChunk* target, const SwtiListType* source, const SwtiListType** out, ImprintAllocator* allocator)
{
    SwtiListType* list = IMPRINT_ALLOC_TYPE(allocator, SwtiListType);
    swtiInitList(list);
    *out = list;
    list->memoryInfo = source->memoryInfo;
    return addType(target, source->itemType, &list->itemType, allocator);
}

static int addType(SwtiChunk* target, const SwtiType* source, const SwtiType** out, ImprintAllocator* allocator)
{
    int foundIndex = swtiChunkFindDeep(target, source);
    if (foundIndex >= 0) {
        *out = target->types[foundIndex];
        return foundIndex;
    }

    if (target->typeCount == target->maxCount) {
        return -3;
    }

    int error = -99;

    switch (source->type) {
        case SwtiTypeAny: {
            SwtiAnyType* any = IMPRINT_ALLOC_TYPE(allocator, SwtiAnyType);
            swtiInitAny(any);
            *out = (const SwtiType*) any;
            break;
        }
        case SwtiTypeAnyMatchingTypes: {
            SwtiAnyMatchingTypesType* anyMatchingTypes = IMPRINT_ALLOC_TYPE(allocator, SwtiAnyMatchingTypesType);
            swtiInitAnyMatchingTypes(anyMatchingTypes);
            *out = (const SwtiType*) anyMatchingTypes;
            break;
        }
        case SwtiTypeCustom: {
            error = addCustomType(target, (const SwtiCustomType*) source, (const SwtiCustomType**) out, allocator);
            break;
        }
        case SwtiTypeFunction: {
            error = addFunction(target, (const SwtiFunctionType*) source, (const SwtiFunctionType**) out, allocator);
            break;
        }
        case SwtiTypeTuple: {
            error = addTuple(target, (const SwtiTupleType*) source, (const SwtiTupleType**) out, allocator);
            break;
        }
        case SwtiTypeAlias: {
            error = addAlias(target, (const SwtiAliasType*) source, (const SwtiAliasType**) out, allocator);
            break;
        }
        case SwtiTypeRecord: {
            error = addRecord(target, (const SwtiRecordType*) source, (const SwtiRecordType**) out, allocator);
            break;
        }
        case SwtiTypeArray: {
            error = addArray(target, (const SwtiArrayType*) source, (const SwtiArrayType**) out, allocator);
            break;
        }
        case SwtiTypeList: {
            error = addList(target, (const SwtiListType*) source, (const SwtiListType**) out, allocator);
            break;
        }
        case SwtiTypeUnmanaged: {
            error = addUnmanaged(target, (const SwtiUnmanagedType*) source, (const SwtiUnmanagedType**) out, allocator);
            break;
        }
        case SwtiTypeString: {
            SwtiStringType* str = IMPRINT_ALLOC_TYPE(allocator, SwtiStringType);
            swtiInitString(str);
            *out = (const SwtiType*) str;
            error = 0;
            break;
        }
        case SwtiTypeResourceName: {
            SwtiResourceNameType* resourceName = IMPRINT_ALLOC_TYPE(allocator, SwtiResourceNameType);
            swtiInitResourceName(resourceName);
            *out = (const SwtiType*) resourceName;
            error = 0;
            break;
        }
        case SwtiTypeChar: {
            SwtiCharType * charType = IMPRINT_ALLOC_TYPE(allocator, SwtiCharType);
            swtiInitChar(charType);
            *out = (const SwtiType*) charType;
            error = 0;
            break;
        }
        case SwtiTypeInt: {
            SwtiIntType* intType = IMPRINT_ALLOC_TYPE(allocator, SwtiIntType);
            swtiInitInt(intType);
            *out = (const SwtiType*) intType;
            error = 0;
            break;
        }
        case SwtiTypeFixed: {
            SwtiFixedType* fixedType = IMPRINT_ALLOC_TYPE(allocator, SwtiFixedType);
            swtiInitFixed(fixedType);
            *out = (const SwtiType*) fixedType;
            error = 0;
            break;
        }
        case SwtiTypeBoolean: {
            SwtiBooleanType* booleanType = IMPRINT_ALLOC_TYPE(allocator, SwtiBooleanType);
            swtiInitBoolean(booleanType);
            *out = (const SwtiType*) booleanType;
            error = 0;
            break;
        }
        case SwtiTypeBlob: {
            SwtiBlobType* blobType = IMPRINT_ALLOC_TYPE(allocator, SwtiBlobType);
            swtiInitBlob(blobType);
            *out = (const SwtiType*) blobType;
            error = 0;
            break;
        }

    }

    if (error < 0) {
        CLOG_ERROR("addType: couldn't consume type information");
        return error;
    }

    int newIndex = target->typeCount++;
    ((SwtiType*) (*out))->index = newIndex;
    target->types[newIndex] = *out;

    return newIndex;
}

int swtiChunkAddType(SwtiChunk* target, const SwtiType* source, ImprintAllocator* allocator)
{
    const SwtiType* ignoreResult;
    return addType(target, source, &ignoreResult, allocator);
}
