/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <flood/in_stream.h>
#include <swamp-typeinfo/chunk.h>
#include <swamp-typeinfo/deserialize.h>
#include <swamp-typeinfo/deserialize_internal.h>
#include <swamp-typeinfo/typeinfo.h>
#include <tiny-libc/tiny_libc.h>

static int readString(FldInStream* stream, const char** outString)
{
    int error;
    uint8_t count;
    if ((error = fldInStreamReadUInt8(stream, &count)) != 0) {
        return error;
    }
    uint8_t* characters = tc_malloc(count + 1);
    if ((error = fldInStreamReadOctets(stream, characters, count)) != 0) {
        return error;
    }
    characters[count] = 0;
    *outString = (const char*) characters;

    return 0;
}

static int readTypeRef(FldInStream* stream, SwtiType** type)
{
    uint8_t index;
    int error;
    if ((error = fldInStreamReadUInt8(stream, &index)) != 0) {
        return error;
    }

    void* fakePointer = (void*) (intptr_t)(int) index;
    *type = fakePointer;

    return 0;
}

static int readTypeRefs(FldInStream* stream, const SwtiType*** outTypes, size_t* outCount)
{
    int error;
    uint8_t count;
    if ((error = fldInStreamReadUInt8(stream, &count)) != 0) {
        *outTypes = 0;
        *outCount = 0;
        return error;
    }

    SwtiType** types = tc_malloc_type_count(SwtiType*, count);
    for (uint8_t i = 0; i < count; i++) {
        if ((error = readTypeRef(stream, &types[i])) != 0) {
            CLOG_ERROR("couldn't read type ref %d", error);
            return error;
        }
    }

    *outTypes = (const SwtiType**) types;
    *outCount = (size_t) count;

    return 0;
}

static int readVariant(FldInStream* stream, SwtiCustomTypeVariant* variant)
{
    const char* name;

    readString(stream, &name);
    variant->name = name;

    return readTypeRefs(stream, &variant->paramTypes, &variant->paramCount);
    ;
}

static int readVariants(FldInStream* stream, SwtiCustomType* custom, uint8_t count)
{
    int error;
    custom->variantTypes = tc_malloc_type_count(SwtiCustomTypeVariant, count);
    for (uint8_t i = 0; i < count; i++) {
        if ((error = readVariant(stream, (SwtiCustomTypeVariant*) &custom->variantTypes[i])) != 0) {
            return error;
        }
    }

    return 0;
}

static int readCustomType(FldInStream* stream, SwtiCustomType** outCustom)
{
    SwtiCustomType* custom = tc_malloc_type(SwtiCustomType);
    swtiInitCustom(custom, 0, 0);
    int error;

    if ((error = readString(stream, &custom->name)) != 0) {
        return error;
    }

    uint8_t variantCount;
    if ((error = fldInStreamReadUInt8(stream, &variantCount)) != 0) {
        return error;
    }
    custom->variantCount = variantCount;

    if ((error = readVariants(stream, custom, variantCount)) != 0) {
        *outCustom = 0;
        return error;
    }

    *outCustom = custom;

    return 0;
}

static int readRecordField(FldInStream* stream, SwtiRecordTypeField* field)
{
    int error;
    if ((error = readString(stream, &field->name)) != 0) {
        return error;
    }

    return readTypeRef(stream, (SwtiType**) &field->fieldType);
}

static int readRecordFields(FldInStream* stream, SwtiRecordType* record, uint8_t count)
{
    int error;
    record->fields = tc_malloc_type_count(SwtiRecordTypeField, count);
    for (uint8_t i = 0; i < count; i++) {
        if ((error = readRecordField(stream, (SwtiRecordTypeField*) &record->fields[i])) != 0) {
            return error;
        }
    }
    record->fieldCount = count;

    return 0;
}

static int readRecord(FldInStream* stream, SwtiRecordType** outRecord)
{
    SwtiRecordType* record = tc_malloc_type(SwtiRecordType);
    swtiInitRecord(record);
    int error;
    uint8_t fieldCount;
    if ((error = fldInStreamReadUInt8(stream, &fieldCount)) != 0) {
        return error;
    }

    if ((error = readRecordFields(stream, record, fieldCount)) != 0) {
        *outRecord = 0;
        return error;
    }

    *outRecord = record;

    return 0;
}

static int readArray(FldInStream* stream, SwtiArrayType** outArray)
{
    SwtiArrayType* array = tc_malloc_type(SwtiArrayType);
    swtiInitArray(array);
    int error;
    if ((error = readTypeRef(stream, (SwtiType**) &array->itemType)) != 0) {
        *outArray = 0;
        return error;
    }

    *outArray = array;
    return 0;
}

static int readList(FldInStream* stream, SwtiListType** outList)
{
    SwtiListType* list = tc_malloc_type(SwtiListType);
    swtiInitList(list);
    int error;
    if ((error = readTypeRef(stream, (SwtiType**) &list->itemType)) != 0) {
        *outList = 0;
        return error;
    }

    *outList = list;
    return 0;
}

static int readFunction(FldInStream* stream, SwtiFunctionType** outFn)
{
    struct SwtiFunctionType* fn = tc_malloc_type(SwtiFunctionType);
    swtiInitFunction(fn, 0, 0);
    int error;
    if ((error = readTypeRefs(stream, &fn->parameterTypes, &fn->parameterCount)) != 0) {
        *outFn = 0;
        return error;
    }

    *outFn = fn;
    return 0;
}

static int readTuple(FldInStream* stream, SwtiTupleType** outTuple)
{
    struct SwtiTupleType* tuple = tc_malloc_type(SwtiTupleType);
    swtiInitTuple(tuple, 0, 0);
    int error;
    if ((error = readTypeRefs(stream, &tuple->parameterTypes, &tuple->parameterCount)) != 0) {
        *outTuple = 0;
        return error;
    }

    *outTuple = tuple;
    return 0;
}

static int readAlias(FldInStream* stream, SwtiAliasType** outAlias)
{
    SwtiAliasType* alias = tc_malloc_type(SwtiAliasType);
    int error;
    if ((error = readString(stream, &alias->internal.name)) != 0) {
        return error;
    }
    alias->internal.type = SwtiTypeAlias;

    if ((error = readTypeRef(stream, (SwtiType**) &alias->targetType)) != 0) {
        *outAlias = 0;
        return error;
    }

    *outAlias = alias;

    return 0;
}

static int readType(FldInStream* stream, const SwtiType** outType)
{
    uint8_t typeValueRaw;
    int error;
    if ((error = fldInStreamReadUInt8(stream, &typeValueRaw)) != 0) {
        CLOG_SOFT_ERROR("readType couldn't read type");
        return error;
    }

    error = -99;
    SwtiTypeValue typeValue = (SwtiTypeValue) typeValueRaw;
    switch (typeValue) {
        case SwtiTypeCustom: {
            SwtiCustomType* custom;
            error = readCustomType(stream, &custom);
            *outType = (const SwtiType*) custom;
            break;
        }
        case SwtiTypeFunction: {
            SwtiFunctionType* fn;
            error = readFunction(stream, &fn);
            *outType = (const SwtiType*) fn;
            break;
        }
        case SwtiTypeAlias: {
            SwtiAliasType* alias;
            error = readAlias(stream, &alias);
            *outType = (const SwtiType*) alias;
            break;
        }
        case SwtiTypeRecord: {
            SwtiRecordType* record;
            error = readRecord(stream, &record);
            *outType = (const SwtiType*) record;
            break;
        }
        case SwtiTypeArray: {
            SwtiArrayType* array;
            error = readArray(stream, &array);
            *outType = (const SwtiType*) array;
            break;
        }
        case SwtiTypeList: {
            SwtiListType* list;
            error = readList(stream, &list);
            *outType = (const SwtiType*) list;
            break;
        }
        case SwtiTypeString: {
            SwtiStringType* string = tc_malloc_type(SwtiStringType);
            swtiInitString(string);
            error = 0;
            *outType = (const SwtiType*) string;
            break;
        }
        case SwtiTypeInt: {
            SwtiIntType* intType = tc_malloc_type(SwtiIntType);
            swtiInitInt(intType);
            error = 0;
            *outType = (const SwtiType*) intType;
            break;
        }
        case SwtiTypeFixed: {
            SwtiFixedType* fixed = tc_malloc_type(SwtiFixedType);
            swtiInitFixed(fixed);
            *outType = (const SwtiType*) fixed;
            error = 0;
            break;
        }
        case SwtiTypeBoolean: {
            SwtiBooleanType* bool = tc_malloc_type(SwtiBooleanType);
            swtiInitBoolean(bool);
            *outType = (const SwtiType*) bool;
            error = 0;
            break;
        }
        case SwtiTypeBlob: {
            SwtiBlobType* blob = tc_malloc_type(SwtiBlobType);
            swtiInitBlob(blob);
            *outType = (const SwtiType*) blob;
            error = 0;
            break;
        }
        case SwtiTypeResourceName: {
            SwtiIntType* intType = tc_malloc_type(SwtiIntType);
            swtiInitInt(intType);
            error = 0;
            *outType = (const SwtiType*) intType;
            break;
        }
        case SwtiTypeChar: {
            SwtiCharType* ch = tc_malloc_type(SwtiCharType);
            swtiInitChar(ch);
            error = 0;
            *outType = (const SwtiType*) ch;
            break;
        }
        case SwtiTypeTuple: {
            SwtiTupleType* tuple;
            error = readTuple(stream, &tuple);
            *outType = (const SwtiType*) tuple;
            break;
        }
        default:
            CLOG_SOFT_ERROR("type information: readType unknown type:%d", typeValue);
            return -14;
    }

    return error;
}

static int deserializeRawFromStream(FldInStream* stream, SwtiChunk* target)
{
    int error;
    uint8_t major;
    uint8_t minor;
    uint8_t patch;

    int tell = stream->pos;

    if ((error = fldInStreamReadUInt8(stream, &major)) != 0) {
        return error;
    }
    if ((error = fldInStreamReadUInt8(stream, &minor)) != 0) {
        return error;
    }
    if ((error = fldInStreamReadUInt8(stream, &patch)) != 0) {
        return error;
    }

    int correct = (major == 0) && (minor == 1) && (patch == 4);
    if (!correct) {
        CLOG_SOFT_ERROR("got wrong version. Expected 0.1.4 and got %d.%d.%d", major, minor, patch)
        return -2;
    }

    uint8_t typesThatFollowCount;
    if ((error = fldInStreamReadUInt8(stream, &typesThatFollowCount)) != 0) {
        return error;
    }

    const struct SwtiType** array = tc_malloc_type_count(const SwtiType*, typesThatFollowCount);
    target->types = array;
    tc_mem_clear_type_n(array, typesThatFollowCount);
    target->typeCount = typesThatFollowCount;

    for (uint8_t i = 0; i < typesThatFollowCount; i++) {
        target->types[i] = 0;
    }

    for (uint8_t i = 0; i < typesThatFollowCount; i++) {
        if ((error = readType(stream, &target->types[i])) != 0) {
            return error;
        }
        ((SwtiType*) (target->types[i]))->index = i;
    }

    int octetsRead = stream->pos - tell;
    return octetsRead;
}

int swtiDeserializeRaw(const uint8_t* octets, size_t octetCount, SwtiChunk* target)
{
    FldInStream stream;

    fldInStreamInit(&stream, octets, octetCount);

    return deserializeRawFromStream(&stream, target);
}

int swtiDeserialize(const uint8_t* octets, size_t octetCount, SwtiChunk* target)
{
    int octetsRead;

    if ((octetsRead = swtiDeserializeRaw(octets, octetCount, target)) < 0) {
        CLOG_SOFT_ERROR("swtiDeserializeRaw %d", octetsRead)
        return octetsRead;
    }

    int error;
    error = swtiDeserializeFixup(target);
    if (error < 0) {
        CLOG_SOFT_ERROR("swtiDeserializeFixup %d", octetsRead)
        return error;
    }

    return octetsRead;
}

int swtiDeserializeFromStream(FldInStream* stream, SwtiChunk* target)
{
    int octetsRead;

    if ((octetsRead = deserializeRawFromStream(stream, target)) < 0) {
        return octetsRead;
    }

    int error;
    error = swtiDeserializeFixup(target);
    if (error < 0) {
        return error;
    }

    return octetsRead;
}
