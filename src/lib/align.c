/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <swamp-typeinfo/typeinfo.h>

SwtiMemoryAlign swtiGetMemoryAlign(const SwtiType* type) {
    switch (type->type) {
        case SwtiTypeRecord: {
            const SwtiRecordType* record = (const SwtiRecordType*) type;
            return record->memoryInfo.memoryAlign;
        } break;
        case SwtiTypeTuple: {
            const SwtiTupleType* tuple = (const SwtiTupleType*) type;
            return tuple->memoryInfo.memoryAlign;
        } break;
        case SwtiTypeList: {
            const SwtiListType* list = (const SwtiListType*) type;
            return list->memoryInfo.memoryAlign;
        } break;
        case SwtiTypeArray: {
            const SwtiArrayType* array = (const SwtiArrayType*) type;
            return array->memoryInfo.memoryAlign;
        } break;
        case SwtiTypeCustom: {
            const SwtiCustomType* custom = (const SwtiCustomType*) type;
            return custom->memoryInfo.memoryAlign;
        } break;
        case SwtiTypeAlias: {
            const SwtiAliasType* alias = (const SwtiAliasType*) type;
            return swtiGetMemoryAlign(alias->targetType);
        } break;
        case SwtiTypeInt: {
            return 4;
        } break;
        case SwtiTypeBoolean: {
            return 1;
        } break;
        default: {
            CLOG_ERROR("can not find alignment for type")
        }
    }
}
