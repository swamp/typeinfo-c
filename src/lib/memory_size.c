/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <swamp-typeinfo/typeinfo.h>

SwtiMemorySize swtiGetMemorySize(const SwtiType* typeToCheck) {
    switch (typeToCheck->type) {
        case SwtiTypeAlias: {
            const SwtiAliasType* alias = (const SwtiAliasType*) typeToCheck;
            return swtiGetMemorySize(alias->targetType);
        }
        case SwtiTypeTuple: {
            const SwtiTupleType* tupleType = (const SwtiTupleType*) typeToCheck;
            return tupleType->memoryInfo.memorySize;
        }
        case SwtiTypeRecord: {
            const SwtiRecordType* recordType = (const SwtiRecordType*) typeToCheck;
            return recordType->memoryInfo.memorySize;
        }
        case SwtiTypeCustom: {
            const SwtiCustomType* customType = (const SwtiCustomType*) typeToCheck;
            return customType->memoryInfo.memorySize;
        }
        case SwtiTypeInt: {
            return 4;
        }
        case SwtiTypeList: {
            const SwtiListType* listType = (const SwtiListType*) typeToCheck;
            return listType->memoryInfo.memorySize;
        }
        default: {
            CLOG_ERROR("do not know memory size");
        }
    }


}
