/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef SWAMP_TYPEINFO_CONSUME_H
#define SWAMP_TYPEINFO_CONSUME_H

struct SwtiChunk;
struct SwtiType;

int swtiTypeConsume(struct SwtiChunk* target, const struct SwtiType* source);

#endif
