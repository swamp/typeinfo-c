/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <swamp-typeinfo/chunk.h>
#include <swamp-typeinfo/typeinfo.h>
#include <swamp-typeinfo/equal.h>

static int typeEqual(const struct SwtiType* a, const struct SwtiType* b);
//static int typeEqual(const struct SwtiType* a, const struct SwtiType* b);

static int typesEqual(const SwtiType** a, const SwtiType** b, size_t count)
{
    int error;

    for (size_t i = 0; i < count; ++i) {
        if ((error = typeEqual(a[i], b[i])) != 0) {
            return error;
        }
    }

    return 0;
}


static int memoryInfoEqual(const SwtiMemoryInfo* a, const SwtiMemoryInfo* b)
{
    if (a->memorySize != b->memorySize) {
        return -5;
    }
    if (a->memoryAlign != b->memoryAlign) {
        return -7;
    }

    return 0;
}

static int memoryOffsetInfoEqual(const SwtiMemoryOffsetInfo* a, const SwtiMemoryOffsetInfo* b)
{
    int result = memoryInfoEqual(&a->memoryInfo, &b->memoryInfo);
    if (result != 0) {
        return result;
    }

    if (a->memoryOffset != b->memoryOffset) {
        return -3;
    }

    return 0;
}

static int variantEqual(const SwtiCustomTypeVariant* a, const SwtiCustomTypeVariant* b)
{
    if (a->paramCount != b->paramCount) {
        return -1;
    }

    if (!tc_str_equal(a->name, b->name)) {
        return -2;
    }

    if (memoryInfoEqual(&a->memoryInfo, &b->memoryInfo) < 0) {
        return -3;
    }

    for (size_t i=0; i<a->paramCount; ++i) {
        if (!typeEqual(a->fields[i].fieldType, b->fields[i].fieldType)) {
            return -3;
        }
        if (a->fields[i].memoryOffsetInfo.memoryOffset != b->fields[i].memoryOffsetInfo.memoryOffset) {
            return -4;
        }
    }

    return 0;
}

static int customEqual(const SwtiCustomType* a, const SwtiCustomType* b)
{
    if (a->variantCount != b->variantCount) {
        return -1;
    }

    int error;
    for (size_t i = 0; i < a->variantCount; ++i) {
        if ((error = variantEqual(&a->variantTypes[i], &b->variantTypes[i])) != 0) {
            return error;
        }
    }

    return 0;
}

static int functionEqual(const SwtiFunctionType* a, const SwtiFunctionType* b)
{
    if (a->parameterCount != b->parameterCount) {
        return -1;
    }

    return typesEqual(a->parameterTypes, b->parameterTypes, a->parameterCount);
}

static int tupleEqual(const SwtiTupleType* a, const SwtiTupleType* b)
{
    if (a->fieldCount != b->fieldCount) {
        return -1;
    }

    if (memoryInfoEqual(&a->memoryInfo, &b->memoryInfo) < 0) {
        return -3;
    }

    return typesEqual(/*todo*/0,0, a->fieldCount);
}

static int aliasEqual(const SwtiAliasType* a, const SwtiAliasType* b)
{
    if (!tc_str_equal(a->internal.name, b->internal.name)) {
        return -1;
    }

    return typeEqual(a->targetType, b->targetType);
}



static int fieldEqual(const SwtiRecordTypeField* a, const SwtiRecordTypeField* b)
{
    if (!tc_str_equal(a->name, b->name)) {
        return -2;
    }

    if (memoryOffsetInfoEqual(&a->memoryOffsetInfo, &b->memoryOffsetInfo) < 0) {
        return -3;
    }

    return typeEqual(a->fieldType, b->fieldType);
}

static int recordEqual(const SwtiRecordType* a, const SwtiRecordType* b)
{
    if (a->fieldCount != b->fieldCount) {
        return -1;
    }

    int error;
    for (size_t i = 0; i < a->fieldCount; ++i) {
        if ((error = fieldEqual(&a->fields[i], &b->fields[i])) != 0) {
            return error;
        }
    }

    return 0;
}

static int arrayEqual(const SwtiArrayType* a, const SwtiArrayType* b)
{
    int memoryEqual = memoryInfoEqual(&a->memoryInfo, &b->memoryInfo);
    if (memoryEqual != 0) {
        return memoryEqual;
    }

    return typeEqual(a->itemType, b->itemType);
}

static int listEqual(const SwtiListType* a, const SwtiListType* b)
{
    int memoryEqual = memoryInfoEqual(&a->memoryInfo, &b->memoryInfo);
    if (memoryEqual != 0) {
        return memoryEqual;
    }

    return typeEqual(a->itemType, b->itemType);
}

static int unmanagedEqual(const SwtiUnmanagedType* a, const SwtiUnmanagedType* b)
{
    if (a->userTypeId != b->userTypeId) {
        return -2;
    }
    if (!tc_str_equal(a->internal.name, b->internal.name)) {
        return -1;
    }

    return 0;
}

static int typeEqual(const struct SwtiType* a, const struct SwtiType* b)
{
    if (a->type != b->type) {
        return -4;
    }

    int error = -99;

    switch (a->type) {
        case SwtiTypeCustom: {
            error = customEqual((const SwtiCustomType*) a, (const SwtiCustomType*) b);
            break;
        }
        case SwtiTypeFunction: {
            error = functionEqual((const SwtiFunctionType*) a, (const SwtiFunctionType*) b);
            break;
        }
        case SwtiTypeTuple: {
            error = tupleEqual((const SwtiTupleType*) a, (const SwtiTupleType*) b);
            break;
        }
        case SwtiTypeAlias: {
            error = aliasEqual((const SwtiAliasType*) a, (const SwtiAliasType*) b);
            break;
        }
        case SwtiTypeRecord: {
            error = recordEqual((const SwtiRecordType*) a, (const SwtiRecordType*) b);
            break;
        }
        case SwtiTypeArray: {
            error = arrayEqual((const SwtiArrayType*) a, (const SwtiArrayType*) b);
            break;
        }
        case SwtiTypeList: {
            error = listEqual((const SwtiListType*) a, (const SwtiListType*) b);
            break;
        }
        case SwtiTypeUnmanaged: {
            error = unmanagedEqual((const SwtiUnmanagedType*) a, (const SwtiUnmanagedType*) b);
        } break;
        case SwtiTypeString: {
            error = 0;
            break;
        }
        case SwtiTypeChar: {
            error = 0;
            break;
        }
        case SwtiTypeResourceName: {
            error = 0;
            break;
        }
        case SwtiTypeInt: {
            error = 0;
            break;
        }
        case SwtiTypeFixed: {
            error = 0;
            break;
        }
        case SwtiTypeBoolean: {
            error = 0;
            break;
        }
        case SwtiTypeBlob: {
            error = 0;
            break;
        }
        default:
            CLOG_ERROR("typeEqual: need information about type %d", a->type)
    }

    return error;
}

/***
 * Checks if two types are equal
 * @param a
 * @param b
 * @return 0 if equal or negative if the types are not equal.
 */
int swtiTypeEqual(const struct SwtiType* a, const struct SwtiType* b)
{
    return typeEqual(a, b);
}
