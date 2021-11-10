/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <swamp-typeinfo/add.h>
#include <swamp-typeinfo/chunk.h>
#include <swamp-typeinfo/typeinfo.h>

static int addType(SwtiChunk* target, const SwtiType* source, const SwtiType** out);

static int addTypes(SwtiChunk* target, const SwtiType** source, const SwtiType*** out, size_t count)
{
    int error;
    *out = 0;
    const SwtiType** targetArray = tc_malloc_type_count(const SwtiType*, count);

    for (size_t i = 0; i < count; ++i) {
        if ((error = addType(target, source[i], &targetArray[i])) < 0) {
            return error;
        }
    }

    *out = targetArray;

    return 0;
}

static int addCustomTypeVariant(SwtiChunk* target, const SwtiCustomTypeVariant* source, SwtiCustomTypeVariant* out)
{
    out->paramCount = source->paramCount;
    out->name = tc_str_dup(source->name);
    out->fields = tc_malloc_type_count(SwtiCustomTypeVariantField, out->paramCount);

    for (size_t i=0; i<out->paramCount; ++i) {
        addType(target, source->fields[i].fieldType, &out->fields[i].fieldType);
        ((SwtiCustomTypeVariantField*)&out->fields[i])->memoryOffsetInfo.memoryOffset = source->fields[i].memoryOffsetInfo.memoryOffset;
    }

    return 0;
}

static int addCustomType(SwtiChunk* target, const SwtiCustomType* source, const SwtiCustomType** out)
{
    SwtiCustomType* custom = tc_malloc_type(SwtiCustomType);
    swtiInitCustom(custom, tc_str_dup(source->internal.name), 0, 0);

    custom->variantTypes = tc_malloc_type_count(SwtiCustomTypeVariant, source->variantCount);
    custom->variantCount = source->variantCount;


    *out = custom;

    int error;
    for (size_t i = 0; i < source->variantCount; ++i) {
        if ((error = addCustomTypeVariant(target, &source->variantTypes[i],
                                          (SwtiCustomTypeVariant*) &custom->variantTypes[i])) != 0) {
            return error;
        }
    }

    return 0;
}

static int addUnmanaged(SwtiChunk* target, const SwtiUnmanagedType* source, const SwtiUnmanagedType** out)
{
    SwtiUnmanagedType* unmanagedType = tc_malloc_type(SwtiUnmanagedType);
    swtiInitUnmanaged(unmanagedType, source->userTypeId, source->internal.name);

    *out = unmanagedType;

    return 0;
}

static int addFunction(SwtiChunk* target, const SwtiFunctionType* source, const SwtiFunctionType** out)
{
    SwtiFunctionType* fn = tc_malloc_type(SwtiFunctionType);
    swtiInitFunction(fn, source->parameterTypes, source->parameterCount);
    *out = fn;

    return addTypes(target, source->parameterTypes, &fn->parameterTypes, source->parameterCount);
}

static int addTuple(SwtiChunk* target, const SwtiTupleType* source, const SwtiTupleType** out)
{
    SwtiTupleType* tuple = tc_malloc_type(SwtiTupleType);
    CLOG_VERBOSE("tuple count %zu", source->fieldCount);
    swtiInitTuple(tuple, 0, source->fieldCount);
    *out = tuple;

    return 0; // addTypes(target, source->parameterTypes, &tuple->parameterTypes, source->parameterCount);
}

static int addAlias(SwtiChunk* target, const SwtiAliasType* source, const SwtiAliasType** out)
{
    SwtiAliasType* alias = tc_malloc_type(SwtiAliasType);
    swtiInitAlias(alias, source->internal.name, 0);
    *out = alias;
    return addType(target, source->targetType, &alias->targetType);
}

static int addField(SwtiChunk* target, const SwtiRecordTypeField* source, SwtiRecordTypeField* out)
{
    out->name = tc_str_dup(source->name);
    return addType(target, source->fieldType, &out->fieldType);
}

static int addRecord(SwtiChunk* target, const SwtiRecordType* source, const SwtiRecordType** out)
{
    SwtiRecordType* record = tc_malloc_type(SwtiRecordType);
    swtiInitRecord(record);
    record->fields = tc_malloc_type_count(SwtiRecordTypeField, source->fieldCount);
    record->fieldCount = source->fieldCount;

    int error;
    for (size_t i = 0; i < source->fieldCount; ++i) {
        if ((error = addField(target, &source->fields[i], (struct SwtiRecordTypeField*) &record->fields[i])) < 0) {
            *out = 0;
            return error;
        }
    }

    *out = record;
    return 0;
}

static int addArray(SwtiChunk* target, const SwtiArrayType* source, const SwtiArrayType** out)
{
    SwtiArrayType* array = tc_malloc_type(SwtiArrayType);
    swtiInitArray(array);
    *out = array;

    return addType(target, source->itemType, &array->itemType);
}

static int addList(SwtiChunk* target, const SwtiListType* source, const SwtiListType** out)
{
    SwtiListType* list = tc_malloc_type(SwtiListType);
    swtiInitList(list);
    *out = list;

    return addType(target, source->itemType, &list->itemType);
}

static int addType(SwtiChunk* target, const SwtiType* source, const SwtiType** out)
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
            SwtiAnyType* any = tc_malloc_type(SwtiAnyType);
            swtiInitAny(any);
            *out = (const SwtiType*) any;
            break;
        }
        case SwtiTypeAnyMatchingTypes: {
            SwtiAnyMatchingTypesType* anyMatchingTypes = tc_malloc_type(SwtiAnyMatchingTypesType);
            swtiInitAnyMatchingTypes(anyMatchingTypes);
            *out = (const SwtiType*) anyMatchingTypes;
            break;
        }
        case SwtiTypeCustom: {
            error = addCustomType(target, (const SwtiCustomType*) source, (const SwtiCustomType**) out);
            break;
        }
        case SwtiTypeFunction: {
            error = addFunction(target, (const SwtiFunctionType*) source, (const SwtiFunctionType**) out);
            break;
        }
        case SwtiTypeTuple: {
            error = addTuple(target, (const SwtiTupleType*) source, (const SwtiTupleType**) out);
            break;
        }
        case SwtiTypeAlias: {
            error = addAlias(target, (const SwtiAliasType*) source, (const SwtiAliasType**) out);
            break;
        }
        case SwtiTypeRecord: {
            error = addRecord(target, (const SwtiRecordType*) source, (const SwtiRecordType**) out);
            break;
        }
        case SwtiTypeArray: {
            error = addArray(target, (const SwtiArrayType*) source, (const SwtiArrayType**) out);
            break;
        }
        case SwtiTypeList: {
            error = addList(target, (const SwtiListType*) source, (const SwtiListType**) out);
            break;
        }
        case SwtiTypeUnmanaged: {
            error = addUnmanaged(target, (const SwtiUnmanagedType*) source, (const SwtiUnmanagedType**) out);
            break;
        }
        case SwtiTypeString: {
            SwtiStringType* str = tc_malloc_type(SwtiStringType);
            swtiInitString(str);
            *out = (const SwtiType*) str;
            error = 0;
            break;
        }
        case SwtiTypeResourceName: {
            SwtiResourceNameType* resourceName = tc_malloc_type(SwtiResourceNameType);
            swtiInitResourceName(resourceName);
            *out = (const SwtiType*) resourceName;
            error = 0;
            break;
        }
        case SwtiTypeChar: {
            SwtiCharType * charType = tc_malloc_type(SwtiCharType);
            swtiInitChar(charType);
            *out = (const SwtiType*) charType;
            error = 0;
            break;
        }
        case SwtiTypeInt: {
            SwtiIntType* intType = tc_malloc_type(SwtiIntType);
            swtiInitInt(intType);
            *out = (const SwtiType*) intType;
            error = 0;
            break;
        }
        case SwtiTypeFixed: {
            SwtiFixedType* fixedType = tc_malloc_type(SwtiFixedType);
            swtiInitFixed(fixedType);
            *out = (const SwtiType*) fixedType;
            error = 0;
            break;
        }
        case SwtiTypeBoolean: {
            SwtiBooleanType* booleanType = tc_malloc_type(SwtiBooleanType);
            swtiInitBoolean(booleanType);
            *out = (const SwtiType*) booleanType;
            error = 0;
            break;
        }
        case SwtiTypeBlob: {
            SwtiBlobType* blobType = tc_malloc_type(SwtiBlobType);
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

int swtiChunkAddType(SwtiChunk* target, const SwtiType* source)
{
    const SwtiType* ignoreResult;
    return addType(target, source, &ignoreResult);
}
