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
        case SwtiTypeCustomVariant: {
            const SwtiCustomTypeVariant * variantType = (const SwtiCustomTypeVariant*) typeToCheck;
            return variantType->memoryInfo.memorySize;
        }
        case SwtiTypeInt: {
            return 4;
        }
        case SwtiTypeRefId: {
            return 4;
        }
        case SwtiTypeChar: {
            return 4;
        }
        case SwtiTypeBoolean: {
            return 1;
        }
        case SwtiTypeList: {
            const SwtiListType* listType = (const SwtiListType*) typeToCheck;
            return 8; //listType->memoryInfo.memorySize;
        }
        case SwtiTypeArray: {
            const SwtiArrayType* arrayType = (const SwtiArrayType*) typeToCheck;
            return 8; //arrayType->memoryInfo.memorySize;
        }
        case SwtiTypeBlob: {
            const SwtiBlobType* blobType = (const SwtiBlobType*) typeToCheck;
            return 8;
        }
        case SwtiTypeUnmanaged: {
          return 8;
        }
        default: {
            CLOG_ERROR("do not know memory size");
        }
    }


}
