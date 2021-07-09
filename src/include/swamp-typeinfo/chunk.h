/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef SWAMP_TYPEINFO_CHUNK_H
#define SWAMP_TYPEINFO_CHUNK_H

#include <stdlib.h>

struct SwtiType;

/***
 * Holds information for all the types for the package.
 */
typedef struct SwtiChunk {
    const struct SwtiType** types;
    size_t typeCount;
    size_t maxCount;
} SwtiChunk;

void swtiChunkInit(SwtiChunk* self, const struct SwtiType** types, size_t typeCount);
void swtiChunkDestroy(SwtiChunk* self);

int swtiChunkFind(const SwtiChunk* self, const struct SwtiType* type);
int swtiChunkFindDeep(const SwtiChunk* self, const struct SwtiType* typeToSearchFor);
int swtiChunkFindFromName(const SwtiChunk* self, const char* typeToSearchFor);
const struct SwtiType* swtiChunkTypeFromIndex(const SwtiChunk* self, size_t index);
const struct SwtiType* swtiChunkGetFromName(const SwtiChunk* self, const char* typeToSearchFor);

int swtiChunkCopy(const SwtiChunk* self, const struct SwtiType* type);
int swtiChunkInitOnlyOneType(SwtiChunk* self, const struct SwtiType *rootType, int* index);

void swtiChunkDebugOutput(const SwtiChunk* self, int flags, const char* debug);

#endif
