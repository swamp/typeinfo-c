/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef SWAMP_TYPEINFO_H
#define SWAMP_TYPEINFO_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct FldOutStream;

// Must be the same as github.com/swamp/compiler/src/typeinfo/typeinfo_serialize.go
typedef enum SwtiTypeValue {
    SwtiTypeCustom,
    SwtiTypeFunction,
    SwtiTypeAlias,
    SwtiTypeRecord,
    SwtiTypeArray,
    SwtiTypeList,
    SwtiTypeString,
    SwtiTypeInt,
    SwtiTypeFixed,
    SwtiTypeBoolean,
    SwtiTypeBlob,
    SwtiTypeResourceName,
    SwtiTypeChar,
    SwtiTypeTuple
} SwtiTypeValue;

typedef struct SwtiType {
    SwtiTypeValue type;
    uint16_t hash;
    uint16_t index;
    const char* name;
} SwtiType;

#define SWTI_TYPE_START(name)                                                                                          \
    typedef struct Swti##name {                                                                                        \
        SwtiType internal;

#define SWTI_TYPE_END(name)                                                                                            \
    }                                                                                                                  \
    Swti##name;

typedef struct SwtiGenericParams {
    size_t genericCount;
    const SwtiType** genericTypes;
} SwtiGenericParams;

typedef struct SwtiCustomTypeVariant {
    size_t paramCount;
    const SwtiType** paramTypes;
    const char* name;
} SwtiCustomTypeVariant;

SWTI_TYPE_START(CustomType)
SwtiGenericParams generic;
size_t variantCount;
const SwtiCustomTypeVariant* variantTypes;
SWTI_TYPE_END(CustomType)

SWTI_TYPE_START(FunctionType)
size_t parameterCount;
const SwtiType** parameterTypes;
SWTI_TYPE_END(FunctionType)

SWTI_TYPE_START(AliasType)
const SwtiType* targetType;
SWTI_TYPE_END(AliasType)

typedef struct SwtiRecordTypeField {
    const SwtiType* fieldType;
    const char* name;
} SwtiRecordTypeField;

SWTI_TYPE_START(RecordType)
SwtiGenericParams generic;
size_t fieldCount;
const SwtiRecordTypeField* fields;
SWTI_TYPE_END(RecordType)

SWTI_TYPE_START(ArrayType)
const SwtiType* itemType;
SWTI_TYPE_END(ArrayType)

SWTI_TYPE_START(ListType)
const SwtiType* itemType;
SWTI_TYPE_END(ListType)

SWTI_TYPE_START(IntType)
SWTI_TYPE_END(IntType)

SWTI_TYPE_START(FixedType)
SWTI_TYPE_END(FixedType)

SWTI_TYPE_START(BooleanType)
SWTI_TYPE_END(BooleanType)

SWTI_TYPE_START(BlobType)
SWTI_TYPE_END(BlobType)

SWTI_TYPE_START(StringType)
SWTI_TYPE_END(StringType)

SWTI_TYPE_START(ResourceNameType)
SWTI_TYPE_END(ResourceNameType)

SWTI_TYPE_START(CharType)
SWTI_TYPE_END(CharType)

SWTI_TYPE_START(TupleType)
    size_t parameterCount;
    const SwtiType** parameterTypes;
SWTI_TYPE_END(TupleType)

void swtiInitString(SwtiStringType* self);
void swtiInitResourceName(SwtiResourceNameType* self);
void swtiInitChar(SwtiCharType* self);
void swtiInitInt(SwtiIntType* self);
void swtiInitFixed(SwtiFixedType* self);
void swtiInitBoolean(SwtiBooleanType* self);
void swtiInitBlob(SwtiBlobType* self);
void swtiInitFunction(SwtiFunctionType* self, const SwtiType* types[], size_t typeCount);
void swtiInitTuple(SwtiTupleType* self, const SwtiType* types[], size_t typeCount);
void swtiInitRecordWithFields(SwtiRecordType* self, const SwtiRecordTypeField fields[], size_t fieldCount);
void swtiInitRecord(SwtiRecordType* self);
void swtiInitAlias(SwtiAliasType* self, const char* name, const SwtiType* targetType);
void swtiInitCustom(SwtiCustomType* self, const char* name,  const SwtiCustomTypeVariant variants[], size_t variantCount);
void swtiInitCustomWithGenerics(SwtiCustomType* self, const char* name, const SwtiType* types[], size_t typeCount,
                                const SwtiCustomTypeVariant variants[], size_t variantCount);

void swtiInitArray(SwtiArrayType* self);
void swtiInitList(SwtiListType* self);
void swtiDebugOutput(struct FldOutStream* fp, const SwtiType* type);
char* swtiDebugString(const SwtiType* type, char* buf, size_t maxCount);

const SwtiType* swtiUnalias(const SwtiType* maybeAlias);
const SwtiListType* swtiList(const SwtiType* maybeAlias);
const SwtiRecordType* swtiRecord(const SwtiType* maybeRecord);

#endif
