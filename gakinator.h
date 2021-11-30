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

static const char gAkinator_Node_modeDescribtion[gAkinator_Node_mode_CNT][MAX_LINE_LEN] = {
        "none",
        "question",
        "answer",
        "unknown",
    };

struct gAkinator_Node
{
    gAkinator_Node_mode mode;
    char question[MAX_LINE_LEN];
    char   answer[MAX_LINE_LEN];
} typedef gAkinator_Node;

typedef gAkinator_Node GTREE_TYPE;

#include "gtree.h"


bool gTree_storeData(gAkinator_Node data, size_t level, FILE *out)  
{
    for (size_t i = 0; i < level; ++i)
        fprintf(out, "\t");
    fprintf(out, "mode=%d\n", data.mode);
    for (size_t i = 0; i < level; ++i)
        fprintf(out, "\t");
    fprintf(out, "question=%s\n", data.question);
    for (size_t i = 0; i < level; ++i)
        fprintf(out, "\t");
    fprintf(out, "answer=%s\n", data.answer);

    return 0;
}

void restoreDataLexer(gAkinator_Node *data, char *buffer)
{
    char *iter = buffer;
    while (isspace(*iter))
        ++iter;

    fprintf(stderr, "scanned buffer = #%s#\n", iter);
    if (!strncmp(iter, "mode=", 5)) {
        fprintf(stderr, "mode scaned!\n");
        iter += 5;
        sscanf(iter, "%d", &data->mode);
    } else if (!strncmp(iter, "question=", 9)) {
        fprintf(stderr, "question scaned!\n");
        iter += 9;
        while (isspace(*iter))
            ++iter;
        strcpy(data->question, iter);
        data->mode = gAkinator_Node_mode_question;
    } else if (!strncmp(iter, "answer=", 7)) {
        fprintf(stderr, "answer scaned!\n");
        iter += 7;
        while (isspace(*iter))
            ++iter;
        strcpy(data->answer, iter);
        data->mode = gAkinator_Node_mode_answer;
    }
    fprintf(stderr, "end of buffer scan\n", iter);
}

bool gTree_restoreData(gAkinator_Node *data, FILE *in)                      
{
    char buffer[MAX_LINE_LEN] = "";
    do {
        if (getline(buffer, MAX_LINE_LEN, in))
            return 1;
        restoreDataLexer(data, buffer);
   } while (!consistsOnly(buffer, "]") && !ferror(in) && !feof(in)); 
   return 0;
}

bool gTree_printData(gAkinator_Node data, FILE *out)
{
    if (data.mode >= gAkinator_Node_mode_CNT || data.mode < 0)
        data.mode = gAkinator_Node_mode_unknown;
    fprintf(out, "{mode | %d (%s)}", data.mode, gAkinator_Node_modeDescribtion[data.mode]);
    switch (data.mode) {
        case (gAkinator_Node_mode_question): 
            fprintf(out, "| {question | %s}", data.question);
            break;
        case (gAkinator_Node_mode_answer): 
            fprintf(out, "| {answer | %s}", data.answer);
            break;
    }
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
