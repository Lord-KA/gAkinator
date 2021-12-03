#pragma once

#include <stdio.h>

static const size_t MAX_LINE_LEN = 1000;

enum gAkinator_Node_mode {
    gAkinator_Node_mode_none, 
    gAkinator_Node_mode_question,
    gAkinator_Node_mode_answer,
    gAkinator_Node_mode_unknown,
    gAkinator_Node_mode_CNT,            
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
    assert(gPtrValid(out));
    assert(level != -1);

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
    assert(gPtrValid(data));
    assert(gPtrValid(buffer));
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
    assert(gPtrValid(data));
    assert(gPtrValid(in));
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
    assert(gPtrValid(out));
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
    gAkinator_status_ObjPoolErr,
    gAkinator_status_FileErr,
    gAkinator_status_BadPtr,
    gAkinator_status_BadId,
    gAkinator_status_BadInput,
    gAkinator_status_CNT,
};

static const char gAkinator_statusMsg[gAkinator_status_CNT][MAX_LINE_LEN] = {
        "OK",
        "Bad structure pointer provided",
        "Error in gTree",
        "Error in gObjPool",
        "Error in file IO",
        "Bad pointer provided",
        "Bad node id provided",
        "WARNING: bad input provided",
    };


#ifndef NLOGS
#define GAKINATOR_ASSERT_LOG(expr, errCode) ASSERT_LOG(expr, errCode, gAkinator_statusMsg[errCode], akinator->logStream)
#else 
#define GAKINATOR_ASSERT_LOG(expr, errCode) ASSERT_LOG(expr, errCode, gAkinator_statusMsg[errCode], NULL)
#endif

#define GAKINATOR_CHECK_SELF_PTR(ptr) ASSERT_LOG(gPtrValid(ptr), gAkinator_status_BadStructPtr,     \
                                                 gAkinator_statusMsg[gAkinator_status_BadStructPtr], \
                                                 stderr)

#define GAKINATOR_NODE_BY_ID(id) ({                                                          \
    gTree_Node *node = NULL;                                                                  \
    GAKINATOR_ASSERT_LOG(gObjPool_get(&akinator->tree.pool, id, &node) == gObjPool_status_OK,  \
                            gAkinator_status_ObjPoolErr);                                       \
    assert(gPtrValid(node));                                                                     \
    node;                                                                                         \
})                                                                   

static const gAkinator_Node FAKE_DATA = {};

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

bool gAkinator_strIsYes(char *buffer) 
{
    assert(gPtrValid(buffer));
    if (!strSkpCmp(buffer, "Yes", 1))
        return 1;
    if (!strSkpCmp(buffer, "YES", 1))
        return 1;
    if (!strSkpCmp(buffer, "yes", 1))
        return 1;
    if (!strSkpCmp(buffer, "Y",   1))
        return 1;
    if (!strSkpCmp(buffer, "y",   1))
        return 1;
    return 0;
}

bool gAkinator_strIsNo(char *buffer) 
{
    assert(gPtrValid(buffer));
    if (!strSkpCmp(buffer, "No", 1))
        return 1;
    if (!strSkpCmp(buffer, "NO", 1))
        return 1;
    if (!strSkpCmp(buffer, "no", 1))
        return 1;
    if (!strSkpCmp(buffer, "N",  1))
        return 1;
    if (!strSkpCmp(buffer, "n",  1))
        return 1;
    return 0;
}

gAkinator_status gAkinator_addNew(gAkinator *akinator, size_t nodeId, gTree_Node *parent);         //TODO remove 

gAkinator_status gAkinator_game(gAkinator *akinator) 
{
    GAKINATOR_CHECK_SELF_PTR(akinator);

    size_t nodeId = akinator->tree.root;
    gTree_Node *node = NULL;
    FILE *out = stdout;
    size_t parentId = -1;
    while (nodeId != -1) {
        node = GAKINATOR_NODE_BY_ID(nodeId);
        if (node->data.mode == gAkinator_Node_mode_question) {
            char answer[MAX_LINE_LEN] = "";
            fprintf(out, "%s (y/n)\n", node->data.question);
            scanf("%s", answer);
            if (gAkinator_strIsNo(answer)) {
                nodeId = node->child;
            } else if (gAkinator_strIsYes(answer)) {
                nodeId = node->child;
                if (nodeId != -1)
                    nodeId = GAKINATOR_NODE_BY_ID(nodeId)->sibling;
            } else {
                fprintf(out, "Unknown option, please try again\n");
            }
        }
        else if (node->data.mode == gAkinator_Node_mode_answer) {
            fprintf(out, "The character is %s (y/n)?\n", node->data.answer);
            char answer[MAX_LINE_LEN] = "";
            scanf("%s", answer);    
            if (gAkinator_strIsNo(answer)) {
                fprintf(out, "We don't know the character, what a shame! But you can add your character right here\n");                //TODO add answer addition mode
                gAkinator_addNew(akinator, nodeId, node);
                goto finish;
            } else if (gAkinator_strIsYes(answer)) {
                fprintf(out, "Great!\nIf you've enjoyed the game, you can by me a coffee\n");        //TODO add coffee buying option
                goto finish;
            } else {
                fprintf(out, "Unknown option, please try again\n");
            }
        } else {
            fprintf(out, "We don't know the character, what a shame! But you can add your character right here\n");                //TODO add answer addition mode
            gAkinator_addNew(akinator, nodeId, node);

            goto finish;
        }
    }
    fprintf(out, "We don't know the character, what a shame! But you can add your character right here\n");                //TODO add answer addition mode
    gAkinator_addNew(akinator, nodeId, node);

finish:

    return gAkinator_status_OK;
}

gAkinator_status gAkinator_definition(const gAkinator *akinator, size_t nodeId) 
{
    GAKINATOR_CHECK_SELF_PTR(akinator);
    GAKINATOR_ASSERT_LOG(nodeId != -1, gAkinator_status_BadId);

    FILE *out = stdout;
    gTree_Node *node = GAKINATOR_NODE_BY_ID(nodeId);

    fprintf(out, "Definition of %s:\n", node->data.answer);
    size_t childId = nodeId;
           nodeId  = node->parent;

    while (childId != akinator->tree.root) {
        gTree_Node *node = GAKINATOR_NODE_BY_ID(nodeId);
        fprintf(out, "%s", node->data.question);
        if (node->child == childId)
            fprintf(out, "\t: Nope!\n");
        else 
            fprintf(out, "\t: Yep! \n");
        childId = nodeId;
        nodeId  = node->parent;
    }

    return gAkinator_status_OK;
}

gAkinator_status gAkinator_addNewQuestion(gAkinator *akinator, size_t &nodeId, size_t &parentId, char question[], char answer[])
{
    FILE *out = stdout;
    gTree_Node *parent = NULL;
    gTree_Node *node   = NULL;
    if (nodeId != -1) {
        node = GAKINATOR_NODE_BY_ID(nodeId);
        if (node->data.mode == gAkinator_Node_mode_none || node->data.mode == gAkinator_Node_mode_unknown) {
            node->data.mode = gAkinator_Node_mode_question;
            strcpy(node->data.question, question);
            size_t childId = -1;
            gTree_Node *child = NULL;
            if (gAkinator_strIsYes(answer)) {
                GAKINATOR_ASSERT_LOG(gTree_addChild(&akinator->tree, nodeId, &childId, node->data) == gTree_status_OK,
                                        gAkinator_status_TreeErr);
            } else if (gAkinator_strIsNo(answer))  {
                size_t siblingId = -1;
                gTree_Node *sibling = NULL;
                GAKINATOR_ASSERT_LOG(gTree_addChild(&akinator->tree, nodeId, &siblingId, node->data) == gTree_status_OK,
                                        gAkinator_status_TreeErr);
                GAKINATOR_ASSERT_LOG(gTree_addChild(&akinator->tree, nodeId, &childId,   node->data) == gTree_status_OK,
                                        gAkinator_status_TreeErr);
                sibling = GAKINATOR_NODE_BY_ID(siblingId);
                sibling->data.mode = gAkinator_Node_mode_none;
            } else {
                fprintf(out, "Bad answer, please try again\n");
                return gAkinator_status_BadInput;
            }
            parentId = nodeId;
            nodeId   = childId;
            gAkinator_addNewQuestion(akinator, nodeId, parentId, question, answer);
        } else {
            parentId = nodeId;
            nodeId   = -1;
            gAkinator_addNewQuestion(akinator, nodeId, parentId, question, answer);
        }
    } else {
        parent = GAKINATOR_NODE_BY_ID(parentId);
        if (parent->child == -1) {
            if (gAkinator_strIsNo(answer)) {
                GAKINATOR_ASSERT_LOG(gTree_addChild(&akinator->tree, parentId, &nodeId, FAKE_DATA) == gTree_status_OK,
                                        gAkinator_status_TreeErr);
            } else if (gAkinator_strIsYes(answer)) {
                size_t siblingId = -1;
                gTree_Node *sibling = NULL;
                GAKINATOR_ASSERT_LOG(gTree_addChild(&akinator->tree, parentId, &siblingId, FAKE_DATA) == gTree_status_OK,
                                        gAkinator_status_TreeErr);
                sibling = GAKINATOR_NODE_BY_ID(siblingId);
                sibling->data.mode     = gAkinator_Node_mode_none;

                GAKINATOR_ASSERT_LOG(gTree_addChild(&akinator->tree, parentId, &nodeId,    FAKE_DATA) == gTree_status_OK,
                                        gAkinator_status_TreeErr);
            } else {
                fprintf(out, "Bad answer, please try again\n");
                return gAkinator_status_BadInput;
            }
        } else {
            GAKINATOR_ASSERT_LOG(gTree_addChild(&akinator->tree, parentId, &nodeId, FAKE_DATA) == gTree_status_OK,
                                    gAkinator_status_TreeErr);
        }
        node = GAKINATOR_NODE_BY_ID(nodeId);
        node->data.mode     = gAkinator_Node_mode_question;
        strcpy(node->data.question, question);
        parentId = nodeId;
        nodeId   = -1;
    }

    return gAkinator_status_OK;
}

gAkinator_status gAkinator_addNew(gAkinator *akinator, size_t nodeId, gTree_Node *parent)       //TODO
{
    GAKINATOR_CHECK_SELF_PTR(akinator);
    GAKINATOR_ASSERT_LOG(gPtrValid(parent), gAkinator_status_BadPtr);

    size_t parentId = -1;
    GAKINATOR_ASSERT_LOG(gObjPool_getId(&akinator->tree.pool, parent, &parentId) == gObjPool_status_OK, gAkinator_status_ObjPoolErr);
    FILE *out = stderr;
    fprintf(out, "Write you characters name:\n");

    char name[MAX_LINE_LEN] = "";
    getline(name, MAX_LINE_LEN, stdin);         //TODO 
    getline(name, MAX_LINE_LEN, stdin);
    
    fprintf(out, "Do you want to add more questions? (y/n)\n");
    char confirmation[MAX_LINE_LEN] = "";
    scanf("%s", confirmation);
    while (gAkinator_strIsYes(confirmation)) {
        char question[MAX_LINE_LEN] = "";
        char   answer[MAX_LINE_LEN] = "";
        fprintf(out, "Your question:\n");
        getline(question, MAX_LINE_LEN, stdin);
        getline(question, MAX_LINE_LEN, stdin);
        fprintf(out, "Your answer (y/n):\n");
        getline(answer,   MAX_LINE_LEN, stdin);

        fprintf(stderr, "question = #%s#\nanswer = #%s#\n", question, answer);

        gAkinator_addNewQuestion(akinator, nodeId, parentId, question, answer);

        fprintf(out, "Add another question? (y/n)\n");
        scanf("%s", confirmation);
    }
    

    return gAkinator_status_OK;
}

gAkinator_status gAkinator_comp(gAkinator *akinator, size_t oneNodeId, size_t otherNodeId)          //TODO
{
    GAKINATOR_CHECK_SELF_PTR(akinator);
    GAKINATOR_ASSERT_LOG(oneNodeId   != -1, gAkinator_status_BadId);
    GAKINATOR_ASSERT_LOG(otherNodeId != -1, gAkinator_status_BadId);

    return gAkinator_status_OK;
}
