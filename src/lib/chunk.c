/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <flood/out_stream.h>
#include <swamp-typeinfo/chunk.h>
#include <swamp-typeinfo/deep_equal.h>
#include <swamp-typeinfo/typeinfo.h>
#include <swamp-typeinfo/consume.h>
#include <tiny-libc/tiny_libc.h>

void swtiChunkInit(SwtiChunk* self, const SwtiType** types, size_t typeCount)
{
    self->types = tc_malloc_type_count(const SwtiType*, typeCount);
    self->typeCount = typeCount;
    tc_memcpy_type(const SwtiType*, self->types, types, typeCount);
}

void swtiDestroyType(SwtiType* type)
{
    switch (type->type) {
        case SwtiTypeCustom:
        case SwtiTypeFunction:
        case SwtiTypeAlias:
        case SwtiTypeRecord:
        case SwtiTypeArray:
        case SwtiTypeList:
        case SwtiTypeString:
        case SwtiTypeInt:
        case SwtiTypeFixed:
        case SwtiTypeBoolean:
        case SwtiTypeBlob:
        case SwtiTypeResourceName:
        case SwtiTypeTuple:
        case SwtiTypeChar:
            break;
    }
    tc_free(type);
}

void swtiChunkDestroy(SwtiChunk* self)
{
    for (size_t i = 0; i < self->typeCount; ++i) {
        swtiDestroyType((SwtiType*) self->types[i]);
    }
    self->types = 0;
    self->typeCount = 0;
    self->maxCount = 0;
}

int swtiChunkFind(const SwtiChunk* self, const SwtiType* typeToSearchFor)
{
    for (size_t i = 0; i < self->typeCount; ++i) {
        const struct SwtiType* type = self->types[i];
        if (type == 0) {
            continue;
        }
        if (typeToSearchFor->hash == type->hash) {
            if (typeToSearchFor->type != type->type) {
                CLOG_SOFT_ERROR("something is wrong here. Hash collide %zu %s vs %s", i, typeToSearchFor->name,
                                type->name);
            }

            return i;
        }
    }

    return -1;
}

int swtiChunkFindFromName(const SwtiChunk* self, const char* typeToSearchFor)
{
    for (size_t i = 0; i < self->typeCount; ++i) {
        const struct SwtiType* type = self->types[i];
        if (type == 0) {
            continue;
        }
        if (tc_str_equal(type->name, typeToSearchFor)) {
            return i;
        }
    }

    return -1;
}

int swtiChunkFindDeep(const SwtiChunk* self, const SwtiType* typeToSearchFor)
{
    for (size_t i = 0; i < self->typeCount; ++i) {
        const struct SwtiType* type = self->types[i];
        if (type == 0) {
            continue;
        }
        if (typeToSearchFor->type == type->type) {
            if (swtiTypeDeepEqual(typeToSearchFor, type) == 0) {
                return i;
            }
        }
    }

    return -1;
}

int swtiChunkInitOnlyOneType(SwtiChunk* targetChunk, const SwtiType *rootType, int* index)
{
    swtiChunkInit(targetChunk, 0, 0);
    targetChunk->maxCount = 8*1024;
    targetChunk->types = tc_malloc_type_count(const SwtiType*, targetChunk->maxCount);

    int rootTypeIndex;
    if ((rootTypeIndex = swtiTypeConsume(targetChunk, rootType)) < 0) {
        *index = rootTypeIndex;
        return rootTypeIndex;
    }

    *index = rootTypeIndex;
    return 0;
}

const struct SwtiType* swtiChunkTypeFromIndex(const SwtiChunk* self, size_t index)
{
    if (index >= self->typeCount) {
        return 0;
    }
    return self->types[index];
}

void swtiChunkDebugOutput(const SwtiChunk* chunk, const char* debug)
{
#define TEMP_BUFFER_SIZE (64 * 1024)
    uint8_t debugOut[TEMP_BUFFER_SIZE];
    FldOutStream stream;
    fldOutStreamInit(&stream, debugOut, TEMP_BUFFER_SIZE);

    fldOutStreamRewind(&stream);
    for (size_t i = 0; i < chunk->typeCount; i++) {
        swtiDebugOutput(&stream, chunk->types[i]);
        fldOutStreamWriteUInt8(&stream, '\n');
    }
    fldOutStreamWriteUInt8(&stream, 0);

    const char* afterFixup = (const char*) debugOut;

    fprintf(stderr, "\n-----%s:\n", debug);
    fputs(afterFixup, stderr);
}
