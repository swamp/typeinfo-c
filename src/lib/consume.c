/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <swamp-typeinfo/chunk.h>
#include <swamp-typeinfo/consume.h>
#include <swamp-typeinfo/typeinfo.h>

static int typeConsume(SwtiChunk* target, const SwtiType* original, const SwtiType** out);

static int typesConsume(SwtiChunk* target, const SwtiType** source, const SwtiType*** out, size_t count)
{
    int error;
    *out = 0;
    const SwtiType** targetArray = tc_malloc_type_count(const SwtiType*, count);

    for (size_t i = 0; i < count; ++i) {
        if ((error = typeConsume(target, source[i], &targetArray[i])) < 0) {
            return error;
        }
    }

    *out = targetArray;

    return 0;
}

static int variantConsume(SwtiChunk* target, const SwtiCustomTypeVariant* source, SwtiCustomTypeVariant* out)
{
    out->paramCount = source->paramCount;
    out->name = tc_str_dup(source->name);

    return typesConsume(target, source->paramTypes, &out->paramTypes, source->paramCount);
}

static int customConsume(SwtiChunk* target, const SwtiCustomType* source, const SwtiCustomType** out)
{
    SwtiCustomType* custom = tc_malloc_type(SwtiCustomType);
    swtiInitCustom(custom, tc_str_dup(source->internal.name), 0, 0);

    custom->variantTypes = tc_malloc_type_count(SwtiCustomTypeVariant, source->variantCount);
    custom->variantCount = source->variantCount;


    *out = custom;

    int error;
    for (size_t i = 0; i < source->variantCount; ++i) {
        if ((error = variantConsume(target, &source->variantTypes[i],
                                    (SwtiCustomTypeVariant*) &custom->variantTypes[i])) != 0) {
            return error;
        }
    }

    return 0;
}

static int functionConsume(SwtiChunk* target, const SwtiFunctionType* source, const SwtiFunctionType** out)
{
    SwtiFunctionType* fn = tc_malloc_type(SwtiFunctionType);
    swtiInitFunction(fn, source->parameterTypes, source->parameterCount);
    *out = fn;

    return typesConsume(target, source->parameterTypes, &fn->parameterTypes, source->parameterCount);
}

static int tupleConsume(SwtiChunk* target, const SwtiTupleType* source, const SwtiTupleType** out)
{
    SwtiTupleType* tuple = tc_malloc_type(SwtiTupleType);
    CLOG_VERBOSE("tuple count %d", source->parameterCount);
    swtiInitTuple(tuple, 0, source->parameterCount);
    *out = tuple;

    return typesConsume(target, source->parameterTypes, &tuple->parameterTypes, source->parameterCount);
}

static int aliasConsume(SwtiChunk* target, const SwtiAliasType* source, const SwtiAliasType** out)
{
    SwtiAliasType* alias = tc_malloc_type(SwtiAliasType);
    swtiInitAlias(alias, source->internal.name, 0);
    *out = alias;
    return typeConsume(target, source->targetType, &alias->targetType);
}

static int fieldConsume(SwtiChunk* target, const SwtiRecordTypeField* source, SwtiRecordTypeField* out)
{
    out->name = tc_str_dup(source->name);
    return typeConsume(target, source->fieldType, &out->fieldType);
}

static int recordConsume(SwtiChunk* target, const SwtiRecordType* source, const SwtiRecordType** out)
{
    SwtiRecordType* record = tc_malloc_type(SwtiRecordType);
    swtiInitRecord(record);
    record->fields = tc_malloc_type_count(SwtiRecordTypeField, source->fieldCount);
    record->fieldCount = source->fieldCount;

    int error;
    for (size_t i = 0; i < source->fieldCount; ++i) {
        if ((error = fieldConsume(target, &source->fields[i], (struct SwtiRecordTypeField*) &record->fields[i])) < 0) {
            *out = 0;
            return error;
        }
    }

    *out = record;
    return 0;
}

static int arrayConsume(SwtiChunk* target, const SwtiArrayType* source, const SwtiArrayType** out)
{
    SwtiArrayType* array = tc_malloc_type(SwtiArrayType);
    swtiInitArray(array);
    *out = array;

    return typeConsume(target, source->itemType, &array->itemType);
}

static int listConsume(SwtiChunk* target, const SwtiListType* source, const SwtiListType** out)
{
    SwtiListType* list = tc_malloc_type(SwtiListType);
    swtiInitList(list);
    *out = list;

    return typeConsume(target, source->itemType, &list->itemType);
}

static int typeConsume(SwtiChunk* target, const SwtiType* source, const SwtiType** out)
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
        case SwtiTypeCustom: {
            error = customConsume(target, (const SwtiCustomType*) source, (const SwtiCustomType**) out);
            break;
        }
        case SwtiTypeFunction: {
            error = functionConsume(target, (const SwtiFunctionType*) source, (const SwtiFunctionType**) out);
            break;
        }
        case SwtiTypeTuple: {
            error = tupleConsume(target, (const SwtiTupleType*) source, (const SwtiTupleType**) out);
            break;
        }
        case SwtiTypeAlias: {
            error = aliasConsume(target, (const SwtiAliasType*) source, (const SwtiAliasType**) out);
            break;
        }
        case SwtiTypeRecord: {
            error = recordConsume(target, (const SwtiRecordType*) source, (const SwtiRecordType**) out);
            break;
        }
        case SwtiTypeArray: {
            error = arrayConsume(target, (const SwtiArrayType*) source, (const SwtiArrayType**) out);
            break;
        }
        case SwtiTypeList: {
            error = listConsume(target, (const SwtiListType*) source, (const SwtiListType**) out);
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
        CLOG_ERROR("couldn't serialize type information");
        return error;
    }

    int newIndex = target->typeCount++;
    ((SwtiType*) (*out))->index = newIndex;
    target->types[newIndex] = *out;

    return newIndex;
}

int swtiTypeConsume(SwtiChunk* target, const SwtiType* source)
{
    const SwtiType* ignoreResult;
    return typeConsume(target, source, &ignoreResult);
}
