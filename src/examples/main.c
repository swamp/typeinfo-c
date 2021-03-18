/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <flood/out_stream.h>
#include <stdio.h>
#include <swamp-typeinfo/chunk.h>
#include <swamp-typeinfo/consume.h>
#include <swamp-typeinfo/deserialize.h>
#include <swamp-typeinfo/deserialize_internal.h>
#include <swamp-typeinfo/serialize.h>
#include <swamp-typeinfo/typeinfo.h>

clog_config g_clog;

static void tyran_log_implementation(enum clog_type type, const char* string)
{
    (void) type;
    fprintf(stderr, "%s\n", string);
}

void test()
{
    SwtiStringType s;
    swtiInitString(&s);

    SwtiIntType integer;
    swtiInitInt(&integer);

    SwtiRecordTypeField a;
    a.fieldType = (const SwtiType*) &s;
    a.name = "firstField";

    SwtiRecordTypeField b;
    b.fieldType = (const SwtiType*) &integer;
    b.name = "second";

    const SwtiRecordTypeField fields[] = {a, b};

    SwtiRecordType r;
    swtiInitRecordWithFields(&r, fields, sizeof(fields) / sizeof(fields[0]));

    SwtiCustomTypeVariant v1;
    v1.paramTypes = 0;
    v1.paramCount = 0;
    v1.name = "Nothing";

    SwtiCustomTypeVariant v2;
    SwtiIntType* ip = &integer;
    v2.paramTypes = (const SwtiType**) &ip;
    v2.paramCount = 1;
    v2.name = "Just";

    const SwtiCustomTypeVariant variants[] = {v2, v1};

    const SwtiType* generics[] = {(const SwtiType*) &integer};
    SwtiCustomType c;
    swtiInitCustomWithGenerics(&c, "Result", generics, 1, variants, sizeof(variants) / sizeof(variants[0]));

    SwtiAliasType maybe;

    swtiInitAlias(&maybe, "Maybe", (const SwtiType*) &c);

    SwtiAliasType alias;

    swtiInitAlias(&alias, "CoolRecord", (const SwtiType*) &r);

    const SwtiType* params[] = {(const SwtiType*) &maybe, (const SwtiType*) &s, (const SwtiType*) &integer};

    SwtiFunctionType fn;
    swtiInitFunction(&fn, params, sizeof(params) / sizeof(params[0]));

    uint8_t debugOut[2048];

    swtiDebugString((const SwtiType*) &fn, debugOut, 2048);
    const char* hack1 = (const char*) debugOut;

    fprintf(stderr, "Constructed:\n%s\n", hack1);

    SwtiChunk chunk;

    const uint8_t octets[] = {0,1,4,0x09,
                              SwtiTypeInt,
                              SwtiTypeList,
                              0x00,
                              SwtiTypeArray,
                              0x04,
                              SwtiTypeAlias,
                              0x4,
                              'C',
                              'o',
                              'o',
                              'l',
                              4,
                              SwtiTypeRecord,
                              2,
                              1,
                              'a',
                              0,
                              1,
                              'b',
                              1,
                              SwtiTypeString,
                              SwtiTypeFunction,
                              2,
                              1,
                              2,
                              SwtiTypeCustom,
                              0x6,
                              'R',
                              'e',
                              's',
                              'u',
                              'l',
                              't',
                              2,
                              5,
                              'M',
                              'a',
                              'y',
                              'b',
                              'e',
                              1,
                              2,
                              3,
                              'N',
                              'o',
                              't',
                              0,
                              SwtiTypeTuple,
                              2,
                              1,0};

    int error = swtiDeserialize(octets, sizeof(octets), &chunk);
    if (error < 0) {
        CLOG_ERROR("problem with deserialization raw typeinformation %d");
    }

    swtiChunkDebugOutput(&chunk, "after deserialization");


    SwtiChunk copyChunk;
    int newIndex;
    int worked = swtiChunkInitOnlyOneType(&copyChunk, chunk.types[8], &newIndex);
    if (worked < 0) {
        CLOG_ERROR("could not make a copy");
    }

    CLOG_INFO("newIndex %d", newIndex);
    swtiChunkDebugOutput(&copyChunk, "after copy");
}


void file() {
    FILE* f = fopen("output.swamp-typeinfo", "rb");
    if (f == 0) {
        CLOG_ERROR("can not find the file");
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* octets = malloc(fsize);
    fread(octets, 1, fsize, f);
    fclose(f);

    SwtiChunk chunk;
    int error = swtiDeserialize(octets, fsize, &chunk);
    if (error != 0) {
        CLOG_ERROR("problem with deserialization raw typeinformation");
    }
    printf("before:-----------------\n");
    swtiChunkDebugOutput(&chunk, "before");

    uint8_t* outOctets = malloc(16 * 1024);

    int octetsWritten = swtiSerialize(outOctets, 16 * 1024, &chunk);
    if (octetsWritten < 0) {
        CLOG_ERROR("problem with serialization typeinformation");
    }

    SwtiChunk deserializedChunk;
    error = swtiDeserialize(outOctets, octetsWritten, &deserializedChunk);
    if (error != 0) {
        CLOG_ERROR("problem with deserialization raw typeinformation");
    }

    printf("WE GOT THIS BACK:-----------------\n");

    swtiChunkDebugOutput(&deserializedChunk, "we got this back");

    SwtiChunk copyChunk;
    copyChunk.typeCount = 0;
    copyChunk.maxCount = 32;
    copyChunk.types = tc_malloc_type_count(SwtiType*, chunk.maxCount);

    error = swtiTypeConsume(&copyChunk, deserializedChunk.types[17]);
    if (error != 0) {
        CLOG_ERROR("problem with consume");
    }
    error = swtiTypeConsume(&copyChunk, deserializedChunk.types[0]);
    if (error != 0) {
        CLOG_ERROR("problem with consume");
    }

    printf("Copied:-----------------\n");

    swtiChunkDebugOutput(&copyChunk, "copied");

}

int main()
{
    g_clog.log = tyran_log_implementation;

    test();

    return 0;
}
