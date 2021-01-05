/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef SWAMP_TYPEINFO_SERIALIZE_H
#define SWAMP_TYPEINFO_SERIALIZE_H

#include <stdint.h>
#include <stdlib.h>

struct SwtiChunk;
struct FldOutStream;

int swtiSerialize(uint8_t* octets, size_t count, const struct SwtiChunk* source);
int swtiSerializeToStream(struct FldOutStream* stream, const struct SwtiChunk* source);

#endif
