/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef SWAMP_TYPEINFO_CHUNK_H
#define SWAMP_TYPEINFO_CHUNK_H

#include <stdlib.h>

struct SwtiType;

typedef struct SwtiChunk {
    const struct SwtiType** types;
    size_t typeCount;
    size_t maxCount;
} SwtiChunk;

void swtiChunkInit(SwtiChunk* self, const struct SwtiType** types, size_t typeCount);
void swtiChunkDestroy(SwtiChunk* self);

int swtiChunkFind(const SwtiChunk* self, const struct SwtiType* type);
int swtiChunkFindDeep(const SwtiChunk* self, const struct SwtiType* typeToSearchFor);
const struct SwtiType* swtiChunkTypeFromIndex(const SwtiChunk* self, size_t index);

int swtiChunkCopy(const SwtiChunk* self, const struct SwtiType* type);

void swtiChunkDebugOutput(const SwtiChunk* self, const char* debug);

#endif
