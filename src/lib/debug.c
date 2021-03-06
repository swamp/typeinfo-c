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
        swtiDebugOutput(fp, sub);
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
        const SwtiType* sub = variant->paramTypes[i];
        swtiDebugOutput(fp, sub);
    }
    fldOutStreamWrites(fp, ")");
}

static void printCustomType(FldOutStream* fp, const SwtiCustomType* custom)
{
    // fldOutStreamWrites(fp, "|");
    // fldOutStreamWritef(fp, "%s ", custom->internal.name);
    fldOutStreamWritef(fp, "%s(", custom->name);
    printGenericParams(fp, &custom->generic);
    if (custom->variantCount == 0) {
        return;
    }
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
        swtiDebugOutput(fp, sub);
    }
    fldOutStreamWrites(fp, ")");
}

static void printAliasType(FldOutStream* fp, const SwtiAliasType* alias)
{
    fldOutStreamWritef(fp, "%s => ", alias->internal.name);
    swtiDebugOutput(fp, alias->targetType);
}

static void printRecordTypeField(FldOutStream* fp, const SwtiRecordTypeField* field)
{
    fldOutStreamWritef(fp, "%s : ", field->name);
    swtiDebugOutput(fp, field->fieldType);
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
    swtiDebugOutput(fp, array->itemType);
    fldOutStreamWrites(fp, ">");
}

static void printListType(FldOutStream* fp, const SwtiListType* list)
{
    fldOutStreamWrites(fp, "List<");
    swtiDebugOutput(fp, list->itemType);
    fldOutStreamWrites(fp, ">");
}

static void printStringType(FldOutStream* fp, const SwtiStringType* string)
{
    fldOutStreamWrites(fp, "String");
}

static void printIntType(FldOutStream* fp, const SwtiIntType* intType)
{
    fldOutStreamWrites(fp, "Int");
}

static void printFixedType(FldOutStream* fp, const SwtiFixedType* fixed)
{
    fldOutStreamWrites(fp, "Fixed");
}

static void printBoolType(FldOutStream* fp, const SwtiBooleanType* fixed)
{
    fldOutStreamWrites(fp, "Bool");
}

static void printBlobType(FldOutStream* fp, const SwtiBlobType* blob)
{
    fldOutStreamWrites(fp, "Blob");
}

void swtiDebugOutput(FldOutStream* fp, const SwtiType* type)
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
            printAliasType(fp, (const SwtiAliasType*) type);
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
        case SwtiTypeBlob:
            printBlobType(fp, (const SwtiBlobType*) type);
            break;
        case SwtiTypeFixed:
            printFixedType(fp, (const SwtiFixedType*) type);
            break;

        default:
            CLOG_ERROR("unknown %d", type->type);
    }
}

const SwtiType* swtiUnalias(const SwtiType* maybeAlias)
{
    if (maybeAlias->type == SwtiTypeAlias) {
        return ((const SwtiAliasType*) maybeAlias)->targetType;
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
