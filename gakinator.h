#pragma once

#include <stdio.h>

static const size_t MAX_LINE_LEN = 100;

enum gAkinator_Node_mode {
    gAkinator_Node_mode_none, 
    gAkinator_Node_mode_question,
    gAkinator_Node_mode_answer,
    gAkinator_Node_mode_unknown,
    gAkinator_Node_mode_CNT,            //TODO think if it is appropriate
};

struct gAkinator_Node
{
    gAkinator_Node_mode mode;
    char question[MAX_LINE_LEN];
    char   answer[MAX_LINE_LEN];
} typedef gAkinator_Node;

typedef gAkinator_Node GTREE_TYPE;
#define GTREE_PRINTF_CODE ""

#include "gtree.h"

bool gTree_storeData(gAkinator_Node data, size_t level, FILE *out)         //TODO
{
    for (size_t i = 0; i < level; ++i)
        fprintf(out, "\t");
    fprintf(out, "mode=%d\n", data.mode);
    for (size_t i = 0; i < level; ++i)
        fprintf(out, "\t");
    fprintf(out, "question=\"%s\"\n", data.question);
    for (size_t i = 0; i < level; ++i)
        fprintf(out, "\t");
    fprintf(out, "answer=\"%s\"\n", data.answer);

    return 0;
}

bool gTree_restoreData(gAkinator_Node *data, FILE *in)                       //TODO
{
    char buffer[MAX_LINE_LEN] = "";

    if (getline(buffer, MAX_LINE_LEN, in))
        return 1;
    if (sscanf(buffer, "mode= %d", &data->mode) != 1)
        return 1;

    fprintf(stderr, "Successfull data read!\n");
    if (getline(buffer, MAX_LINE_LEN, in))
        return 1;
    if (sscanf(buffer, "question=\" %s \"", &data->question) != 1)
        return 1;

    if (getline(buffer, MAX_LINE_LEN, in))
        return 1;
    if (sscanf(buffer, "answer=\" %s \"", &data->answer) != 1)
        return 1;
    
    return 0;
}

enum gAkinator_status {
    gAkinator_status_OK = 0,
    gAkinator_status_BadStructPtr,
    gAkinator_status_TreeErr,
    gAkinator_status_FileErr,
    gAkinator_status_CNT,
};

static const char gAkinator_statusMsg[gAkinator_status_CNT][MAX_LINE_LEN] = {
        "OK",
        "Bad structure pointer provided",
        "Error in gTree",
        "Error in file IO",
    };

#ifndef NLOGS
#define GAKINATOR_ASSERT_LOG(expr, errCode) ASSERT_LOG(expr, errCode, gAkinator_statusMsg[errCode], akinator->logStream)
#else 
#define GAKINATOR_ASSERT_LOG(expr, errCode) ASSERT_LOG(expr, errCode, gAkinator_statusMsg[errCode], NULL)
#endif

#define GAKINATOR_CHECK_SELF_PTR(ptr) ASSERT_LOG(gPtrValid(ptr), gAkinator_status_BadStructPtr, \
                                                 gAkinator_statusMsg[gAkinator_status_BadStructPtr], \
                                                 stderr)

struct gAkinator {
    gTree tree;
    FILE *logStream;
} typedef gAkinator;


gAkinator_status gAkinator_ctor(gAkinator *akinator, FILE *newLogStream, FILE *storage) 
{
    if (!gPtrValid(akinator)) {                                          
        FILE *out;                                                   
        if (!gPtrValid(newLogStream)) 
            out = stderr;                                            
        else                                                         
            out = newLogStream;                                      
        fprintf(out, "ERROR: bad structure ptr provided to akinator ctor!\n");
        return gAkinator_status_BadStructPtr;                         
    }

    akinator->logStream = stderr;
    if (gPtrValid(newLogStream))
        akinator->logStream = newLogStream;

    gTree_status status = gTree_restoreTree(&akinator->tree, newLogStream, storage);
    GAKINATOR_ASSERT_LOG(status == gTree_status_OK, gAkinator_status_TreeErr);

    return gAkinator_status_OK;
}

gAkinator_status gAkinator_dtor(gAkinator *akinator) 
{
    GAKINATOR_CHECK_SELF_PTR(akinator);

    gTree_status status = gTree_dtor(&akinator->tree);
    GAKINATOR_ASSERT_LOG(status == gTree_status_OK, gAkinator_status_TreeErr);

    return gAkinator_status_OK;
}
