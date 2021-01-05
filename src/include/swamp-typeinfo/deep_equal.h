/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef SWAMP_TYPEINFO_DEEP_EQUAL_H
#define SWAMP_TYPEINFO_DEEP_EQUAL_H

struct SwtiType;

int swtiTypeDeepEqual(const struct SwtiType* a, const struct SwtiType* b);

#endif
