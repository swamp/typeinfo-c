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
struct ImprintAllocator;

typedef enum SwtiDebugOutputFlags {
    SwtiDebugOutputFlagsExpandAlias = 1
} SwtiDebugOutputFlags;

// Must be the same as github.com/swamp/compiler/src/typeinfo/typeinfo_serialize.go
typedef enum SwtiTypeValue {
    SwtiTypeCustom,
    SwtiTypeCustomVariant,
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
    SwtiTypeTuple,
    SwtiTypeRefId,
    SwtiTypeAny,
    SwtiTypeAnyMatchingTypes,
    SwtiTypeUnmanaged
} SwtiTypeValue;

typedef uint16_t SwtiMemoryOffset;
typedef uint16_t SwtiMemorySize;
typedef uint8_t SwtiMemoryAlign;

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

typedef struct SwtiMemoryInfo {
    SwtiMemorySize memorySize;
    SwtiMemoryAlign memoryAlign;
} SwtiMemoryInfo;

typedef struct SwtiMemoryOffsetInfo {
    SwtiMemoryOffset memoryOffset;
    SwtiMemoryInfo memoryInfo;
} SwtiMemoryOffsetInfo;

typedef struct SwtiGenericParams {
    size_t genericCount;
    const SwtiType** genericTypes;
} SwtiGenericParams;

typedef struct SwtiCustomTypeVariantField {
    const SwtiType* fieldType;
    SwtiMemoryOffsetInfo memoryOffsetInfo;
} SwtiCustomTypeVariantField;

struct SwtiCustomType;

SWTI_TYPE_START(CustomTypeVariant)
    const struct SwtiCustomType* inCustomType;
    uint8_t paramCount;
    const SwtiCustomTypeVariantField* fields;
    const char* name;
    SwtiMemoryInfo memoryInfo;
SWTI_TYPE_END(CustomTypeVariant)

SWTI_TYPE_START(CustomType)
SwtiGenericParams generic;
size_t variantCount;
const SwtiCustomTypeVariant** variantTypes;
SwtiMemoryInfo memoryInfo;
SWTI_TYPE_END(CustomType)

typedef struct SwtiTupleTypeField {
    const SwtiType* fieldType;
    SwtiMemoryOffsetInfo memoryOffsetInfo;
    const char* name;
} SwtiTupleTypeField;

SWTI_TYPE_START(TupleType)
size_t fieldCount;
const SwtiTupleTypeField* fields;
SwtiMemoryInfo memoryInfo;
SWTI_TYPE_END(TupleType)

SWTI_TYPE_START(FunctionType)
size_t parameterCount;
const SwtiType** parameterTypes;
SWTI_TYPE_END(FunctionType)

SWTI_TYPE_START(AliasType)
const SwtiType* targetType;
SWTI_TYPE_END(AliasType)

SWTI_TYPE_START(TypeRefIdType)
const SwtiType* referencedType;
SWTI_TYPE_END(TypeRefIdType)

typedef struct SwtiRecordTypeField {
    const SwtiType* fieldType;
    SwtiMemoryOffsetInfo memoryOffsetInfo;
    const char* name;
} SwtiRecordTypeField;

SWTI_TYPE_START(RecordType)
SwtiGenericParams generic;
size_t fieldCount;
const SwtiRecordTypeField* fields;
SwtiMemoryInfo memoryInfo;
SWTI_TYPE_END(RecordType)

SWTI_TYPE_START(ArrayType)
const SwtiType* itemType;
SwtiMemoryInfo memoryInfo;
SWTI_TYPE_END(ArrayType)

SWTI_TYPE_START(ListType)
const SwtiType* itemType;
SwtiMemoryInfo memoryInfo;
SWTI_TYPE_END(ListType)



SWTI_TYPE_START(IntType)
SWTI_TYPE_END(IntType)

SWTI_TYPE_START(AnyType)
SWTI_TYPE_END(AnyType)

SWTI_TYPE_START(UnmanagedType)
uint16_t userTypeId;
SWTI_TYPE_END(UnmanagedType)

SWTI_TYPE_START(AnyMatchingTypesType)
SWTI_TYPE_END(AnyMatchingTypesType)

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




void swtiInitString(SwtiStringType* self);
void swtiInitResourceName(SwtiResourceNameType* self);
void swtiInitChar(SwtiCharType* self);
void swtiInitTypeRefId(SwtiTypeRefIdType * self, const SwtiType* targetType);
void swtiInitInt(SwtiIntType* self);
void swtiInitAny(SwtiAnyType* self);
void swtiInitUnmanaged(SwtiUnmanagedType* self, uint16_t userTypeId, const char* name, struct ImprintAllocator* allocator);
void swtiInitFixed(SwtiFixedType* self);
void swtiInitBoolean(SwtiBooleanType* self);
void swtiInitAnyMatchingTypes(SwtiAnyMatchingTypesType * self);
void swtiInitBlob(SwtiBlobType* self);
void swtiInitFunction(SwtiFunctionType* self, const SwtiType* types[], size_t typeCount, struct ImprintAllocator* allocator);
void swtiInitTuple(SwtiTupleType* self, const SwtiTupleTypeField types[], size_t typeCount, struct ImprintAllocator* allocator);
void swtiInitRecordWithFields(SwtiRecordType* self, const SwtiRecordTypeField fields[], size_t fieldCount, struct ImprintAllocator* allocator);
void swtiInitRecord(SwtiRecordType* self);
void swtiInitAlias(SwtiAliasType* self, const char* name, const SwtiType* targetType);
void swtiInitCustom(SwtiCustomType* self, const char* name,  const SwtiCustomTypeVariant variants[], size_t variantCount, struct ImprintAllocator* allocator);
void swtiInitCustomWithGenerics(SwtiCustomType* self, const char* name, const SwtiType* types[], size_t typeCount,
                                const SwtiCustomTypeVariant variants[], size_t variantCount, struct ImprintAllocator* allocator);
void swtiInitVariant(SwtiCustomTypeVariant* self, const SwtiCustomTypeVariantField sourceFields[], size_t typeCount, struct ImprintAllocator* allocator);
void swtiInitArray(SwtiArrayType* self);
void swtiInitList(SwtiListType* self);
void swtiDebugOutput(struct FldOutStream* fp, SwtiDebugOutputFlags flags, const SwtiType* type);
char* swtiDebugString(const SwtiType* type, SwtiDebugOutputFlags flags, char* buf, size_t maxCount);

const SwtiType* swtiUnalias(const SwtiType* maybeAlias);
const SwtiListType* swtiList(const SwtiType* maybeAlias);
const SwtiRecordType* swtiRecord(const SwtiType* maybeRecord);
SwtiMemoryAlign swtiGetMemoryAlign(const SwtiType* type);
SwtiMemorySize swtiGetMemorySize(const SwtiType* type);

int swtiVerifyMemoryInfo(const SwtiMemoryInfo* info);
int swtiVerifyMemoryOffsetInfo(const SwtiMemoryOffsetInfo* info);
int swtiVerifyTuple(const SwtiTupleType* tuple);

#endif
