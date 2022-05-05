/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <flood/out_stream.h>
#include <stdio.h>
#include <swamp-typeinfo/typeinfo.h>

static void printGenericParams(FldOutStream* fp, const SwtiGenericParams* params)
{
    if (params->genericCount == 0) {
        return;
    }

    fldOutStreamWrites(fp, "<");
    for (size_t i = 0; i < params->genericCount; i++) {
        if (i > 0) {
            fldOutStreamWrites(fp, " -> ");
        }
        const SwtiType* sub = params->genericTypes[i];
        swtiDebugOutput(fp, 0, sub);
    }
    fldOutStreamWrites(fp, ">");
}

static void printCustomTypeVariant(FldOutStream* fp, const SwtiCustomTypeVariant* variant)
{
    fldOutStreamWritef(fp, "%s", variant->name);
    if (variant->paramCount == 0) {
        return;
    }
    fldOutStreamWrites(fp, "(");
    for (size_t i = 0; i < variant->paramCount; i++) {
        if (i > 0) {
            fldOutStreamWrites(fp, ", ");
        }
        const SwtiType* sub = variant->fields[i].fieldType;
        swtiDebugOutput(fp, 0, sub);
    }
    fldOutStreamWrites(fp, ")");
}

static void printCustomType(FldOutStream* fp, const SwtiCustomType* custom)
{
    // fldOutStreamWrites(fp, "|");
    // fldOutStreamWritef(fp, "%s ", custom->internal.name);
    fldOutStreamWritef(fp, "%s", custom->internal.name);
    printGenericParams(fp, &custom->generic);
    if (custom->variantCount == 0) {
        return;
    }
    fldOutStreamWritef(fp, "(");
    for (size_t i = 0; i < custom->variantCount; i++) {
        if (i > 0) {
            fldOutStreamWrites(fp, " | ");
        }
        const SwtiCustomTypeVariant* sub = &custom->variantTypes[i];
        printCustomTypeVariant(fp, sub);
    }
    fldOutStreamWrites(fp, ")");
}

static void printFunctionType(FldOutStream* fp, const SwtiFunctionType* fn)
{
    fldOutStreamWrites(fp, "(");
    for (size_t i = 0; i < fn->parameterCount; i++) {
        if (i > 0) {
            fldOutStreamWrites(fp, " -> ");
        }
        const SwtiType* sub = fn->parameterTypes[i];
        swtiDebugOutput(fp, 0, sub);
    }
    fldOutStreamWrites(fp, ")");
}

static void printTupleType(FldOutStream* fp, const SwtiTupleType * fn)
{
    fldOutStreamWrites(fp, "(");
    for (size_t i = 0; i < fn->fieldCount; i++) {
        if (i > 0) {
            fldOutStreamWrites(fp, ", ");
        }
        const SwtiTupleTypeField* sub = &fn->fields[i];
        swtiDebugOutput(fp, 0, sub->fieldType);
    }
    fldOutStreamWrites(fp, ")");
}

static void printAliasType(FldOutStream* fp, SwtiDebugOutputFlags flags, const SwtiAliasType* alias)
{
    if (flags & SwtiDebugOutputFlagsExpandAlias) {
        fldOutStreamWritef(fp, "%s => ", alias->internal.name);
        swtiDebugOutput(fp, flags, alias->targetType);
    } else {
        fldOutStreamWritef(fp, "%s", alias->internal.name);
    }
}

static void printTypeRefIdType(FldOutStream* fp, SwtiDebugOutputFlags flags, const SwtiTypeRefIdType* typeRefId)
{
    fldOutStreamWritef(fp, "$%s", typeRefId->referencedType->name);
}




static void printRecordTypeField(FldOutStream* fp, const SwtiRecordTypeField* field)
{
    fldOutStreamWritef(fp, "%s : ", field->name);
    swtiDebugOutput(fp, 0, field->fieldType);
}

static void printRecordType(FldOutStream* fp, const SwtiRecordType* record)
{
    fldOutStreamWrites(fp, "{");
    for (size_t i = 0; i < record->fieldCount; i++) {
        if (i > 0) {
            fldOutStreamWrites(fp, ", ");
        }
        const SwtiRecordTypeField* field = &record->fields[i];
        printRecordTypeField(fp, field);
    }
    fldOutStreamWrites(fp, "}");
}

static void printArrayType(FldOutStream* fp, const SwtiArrayType* array)
{
    fldOutStreamWrites(fp, "Array<");
    swtiDebugOutput(fp, 0, array->itemType);
    fldOutStreamWrites(fp, ">");
}

static void printListType(FldOutStream* fp, const SwtiListType* list)
{
    fldOutStreamWrites(fp, "List<");
    swtiDebugOutput(fp, 0, list->itemType);
    fldOutStreamWrites(fp, ">");
}

static void printStringType(FldOutStream* fp, const SwtiStringType* string)
{
    fldOutStreamWrites(fp, "String");
}

static void printCharType(FldOutStream* fp, const SwtiCharType* ch)
{
    fldOutStreamWrites(fp, "Char");
}

static void printUnmanagedType(FldOutStream* fp, const SwtiUnmanagedType* unmanaged)
{
    fldOutStreamWritef(fp, "Unmanaged<%s>", unmanaged->internal.name);
}

static void printIntType(FldOutStream* fp, const SwtiIntType* intType)
{
    fldOutStreamWrites(fp, "Int");
}

static void printFixedType(FldOutStream* fp, const SwtiFixedType* fixed)
{
    fldOutStreamWrites(fp, "Fixed");
}

static void printBoolType(FldOutStream* fp, const SwtiBooleanType* boolType)
{
    fldOutStreamWrites(fp, "Bool");
}

static void printAnyType(FldOutStream* fp, const SwtiAnyType* any)
{
    fldOutStreamWrites(fp, "Any");
}

static void printAnyMatchingTypesType(FldOutStream* fp, const SwtiAnyMatchingTypesType* anyMatching)
{
    fldOutStreamWrites(fp, "*");
}

static void printBlobType(FldOutStream* fp, const SwtiBlobType* blob)
{
    fldOutStreamWrites(fp, "Blob");
}

void swtiDebugOutput(FldOutStream* fp, SwtiDebugOutputFlags flags, const SwtiType* type)
{
    uintptr_t ptrValue = (uintptr_t)(void*) type;
    if (ptrValue < 256) {
        fldOutStreamWritef(fp, "reference: %d", ptrValue);
        return;
    }
    switch (type->type) {
        case SwtiTypeCustom:
            printCustomType(fp, (const SwtiCustomType*) type);
            break;
        case SwtiTypeFunction:
            printFunctionType(fp, (const SwtiFunctionType*) type);
            break;
        case SwtiTypeAlias:
            printAliasType(fp, flags, (const SwtiAliasType*) type);
            break;
        case SwtiTypeRefId:
            printTypeRefIdType(fp, flags, (const SwtiTypeRefIdType *) type);
            break;
        case SwtiTypeRecord:
            printRecordType(fp, (const SwtiRecordType*) type);
            break;
        case SwtiTypeArray:
            printArrayType(fp, (const SwtiArrayType*) type);
            break;
        case SwtiTypeList:
            printListType(fp, (const SwtiListType*) type);
            break;
        case SwtiTypeString:
            printStringType(fp, (const SwtiStringType*) type);
            break;
        case SwtiTypeInt:
            printIntType(fp, (const SwtiIntType*) type);
            break;
        case SwtiTypeBoolean:
            printBoolType(fp, (const SwtiBooleanType*) type);
            break;
        case SwtiTypeAny:
            printAnyType(fp, (const SwtiAnyType *) type);
            break;
        case SwtiTypeAnyMatchingTypes:
            printAnyMatchingTypesType(fp, (const SwtiAnyMatchingTypesType *) type);
            break;
        case SwtiTypeBlob:
            printBlobType(fp, (const SwtiBlobType*) type);
            break;
        case SwtiTypeFixed:
            printFixedType(fp, (const SwtiFixedType*) type);
            break;
        case SwtiTypeTuple:
            printTupleType(fp, (const SwtiTupleType*) type);
            break;
        case SwtiTypeChar:
            printCharType(fp, (const SwtiCharType*) type);
            break;
        case SwtiTypeUnmanaged:
            printUnmanagedType(fp, (const SwtiUnmanagedType*) type);
            break;
        case SwtiTypeResourceName:
            fldOutStreamWritef(fp, "resource name %p", (const SwtiResourceNameType*) type);
            break;
        default:
            CLOG_ERROR("swtidebugoutput unknown %d", type->type)
    }
}

char* swtiDebugString(const SwtiType* type, SwtiDebugOutputFlags flags, char* buf, size_t maxBuf)
{
    FldOutStream outStream;

    fldOutStreamInit(&outStream, (uint8_t *) buf, maxBuf);

    swtiDebugOutput(&outStream, flags, type);

    fldOutStreamWriteUInt8(&outStream, 0);

    return buf;
}

const SwtiType* swtiUnalias(const SwtiType* maybeAlias)
{
    if (maybeAlias->type == SwtiTypeAlias) {
        return swtiUnalias(((const SwtiAliasType*) maybeAlias)->targetType);
    }

    return maybeAlias;
}

const SwtiListType* swtiList(const SwtiType* maybeList)
{
    if (maybeList->type == SwtiTypeList) {
        return (const SwtiListType*) maybeList;
    }

    return 0;
}

const SwtiRecordType* swtiRecord(const SwtiType* maybeRecord)
{
    if (maybeRecord->type == SwtiTypeRecord) {
        return (const SwtiRecordType*) maybeRecord;
    }

    return 0;
}
