/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <swamp-typeinfo/chunk.h>
#include <swamp-typeinfo/typeinfo.h>

static int typeDeepEqual(const struct SwtiType* a, const struct SwtiType* b);
static int typeDeepEqual(const struct SwtiType* a, const struct SwtiType* b);

static int typesDeepEqual(const SwtiType** a, const SwtiType** b, size_t count)
{
    int error;

    for (size_t i = 0; i < count; ++i) {
        if ((error = typeDeepEqual(a[i], b[i])) != 0) {
            return error;
        }
    }

    return 0;
}

static int variantDeepEqual(const SwtiCustomTypeVariant* a, const SwtiCustomTypeVariant* b)
{
    if (a->paramCount != b->paramCount) {
        return -1;
    }

    if (!tc_str_equal(a->name, b->name)) {
        return -2;
    }

    return typesDeepEqual(a->paramTypes, b->paramTypes, a->paramCount);
}

static int customDeepEqual(const SwtiCustomType* a, const SwtiCustomType* b)
{
    if (a->variantCount != b->variantCount) {
        return -1;
    }

    int error;
    for (size_t i = 0; i < a->variantCount; ++i) {
        if ((error = variantDeepEqual(&a->variantTypes[i], &b->variantTypes[i])) != 0) {
            return error;
        }
    }

    return 0;
}

static int functionDeepEqual(const SwtiFunctionType* a, const SwtiFunctionType* b)
{
    if (a->parameterCount != b->parameterCount) {
        return -1;
    }

    return typesDeepEqual(a->parameterTypes, b->parameterTypes, a->parameterCount);
}

static int aliasDeepEqual(const SwtiAliasType* a, const SwtiAliasType* b)
{
    if (!tc_str_equal(a->internal.name, b->internal.name)) {
        return -1;
    }

    return typeDeepEqual(a->targetType, b->targetType);
}

static int fieldDeepEqual(const SwtiRecordTypeField* a, const SwtiRecordTypeField* b)
{
    if (!tc_str_equal(a->name, b->name)) {
        return -2;
    }

    return typeDeepEqual(a->fieldType, b->fieldType);
}

static int recordDeepEqual(const SwtiRecordType* a, const SwtiRecordType* b)
{
    if (a->fieldCount != b->fieldCount) {
        return -1;
    }

    int error;
    for (size_t i = 0; i < a->fieldCount; ++i) {
        if ((error = fieldDeepEqual(&a->fields[i], &b->fields[i])) != 0) {
            return error;
        }
    }

    return 0;
}

static int arrayDeepEqual(const SwtiArrayType* a, const SwtiArrayType* b)
{
    return typeDeepEqual(a->itemType, b->itemType);
}

static int listDeepEqual(const SwtiListType* a, const SwtiListType* b)
{
    return typeDeepEqual(a->itemType, b->itemType);
}

static int typeDeepEqual(const struct SwtiType* a, const struct SwtiType* b)
{
    if (a->type != b->type) {
        return -4;
    }

    int error = -99;

    switch (a->type) {
        case SwtiTypeCustom: {
            error = customDeepEqual((const SwtiCustomType*) a, (const SwtiCustomType*) b);
            break;
        }
        case SwtiTypeFunction: {
            error = functionDeepEqual((const SwtiFunctionType*) a, (const SwtiFunctionType*) b);
            break;
        }
        case SwtiTypeAlias: {
            error = aliasDeepEqual((const SwtiAliasType*) a, (const SwtiAliasType*) b);
            break;
        }
        case SwtiTypeRecord: {
            error = recordDeepEqual((const SwtiRecordType*) a, (const SwtiRecordType*) b);
            break;
        }
        case SwtiTypeArray: {
            error = arrayDeepEqual((const SwtiArrayType*) a, (const SwtiArrayType*) b);
            break;
        }
        case SwtiTypeList: {
            error = listDeepEqual((const SwtiListType*) a, (const SwtiListType*) b);
            break;
        }
        case SwtiTypeString: {
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
    }

    return error;
}

int swtiTypeDeepEqual(const struct SwtiType* a, const struct SwtiType* b)
{
    return typeDeepEqual(a, b);
}
