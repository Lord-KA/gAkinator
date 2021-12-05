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

    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "scanned buffer = #%s#\n", iter);
    #endif
    if (!strncmp(iter, "mode=", 5)) {
        #ifdef EXTRA_VERBOSE
            fprintf(stderr, "mode scaned!\n");
        #endif
        iter += 5;
        sscanf(iter, "%d", &data->mode);
    } else if (!strncmp(iter, "question=", 9)) {
        #ifdef EXTRA_VERBOSE
            fprintf(stderr, "question scaned!\n");
        #endif
        iter += 9;
        while (isspace(*iter))
            ++iter;
        strcpy(data->question, iter);
        data->mode = gAkinator_Node_mode_question;
    } else if (!strncmp(iter, "answer=", 7)) {
        #ifdef EXTRA_VERBOSE
            fprintf(stderr, "answer scaned!\n");
        #endif
        iter += 7;
        while (isspace(*iter))
            ++iter;
        strcpy(data->answer, iter);
        data->mode = gAkinator_Node_mode_answer;
    }
    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "end of buffer scan\n", iter);
    #endif
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

gAkinator_status gAkinator_addNew(gAkinator *akinator, size_t nodeId, gTree_Node *parent)
{
    GAKINATOR_CHECK_SELF_PTR(akinator);
    GAKINATOR_ASSERT_LOG(gPtrValid(parent), gAkinator_status_BadPtr);

    size_t parentId = -1;
    GAKINATOR_ASSERT_LOG(gObjPool_getId(&akinator->tree.pool, parent, &parentId) == gObjPool_status_OK, gAkinator_status_ObjPoolErr);
    FILE *out = stdout;
    char question[MAX_LINE_LEN] = "";
    char   answer[MAX_LINE_LEN] = "";
    do {
        fprintf(out, "Do you want to add a character (y/n)?\n");
        assert(!getline(answer, MAX_LINE_LEN, stdin));
        if (strIsNo(answer))
            return gAkinator_status_OK;
        if (!strIsYes(answer))
            fprintf(out, "Bad answer, please try again\n");
    } while (!strIsYes(answer));
    fprintf(out, "Write you characters name:\n");

    char name[MAX_LINE_LEN] = "";
    assert(!getline(name, MAX_LINE_LEN, stdin));
 
    gTree_Node *node = NULL;
    if (nodeId != -1) {
        node = GAKINATOR_NODE_BY_ID(nodeId);
        if (node->data.mode == gAkinator_Node_mode_answer) {
            fprintf(out, "How whould you differ %s from %s?\n", node->data.answer, name);
            fprintf(out, "Your question:\n");
            assert(!getline(question, MAX_LINE_LEN, stdin));
            fprintf(out, "Whould you say it about %s (y/n):\n", name);
            assert(!getline(answer,   MAX_LINE_LEN, stdin));
            while (!strIsYes(answer) && !strIsNo(answer)) {
                fprintf(out, "Bad answer, please try again\n");
                fprintf(out, "Whould you say it about %s (y/n):\n", name);
                assert(!getline(answer,   MAX_LINE_LEN, stdin));
            }

            size_t yChildId = -1, nChildId = -1;
            GAKINATOR_ASSERT_LOG(gTree_addChild(&akinator->tree, nodeId, &nChildId, FAKE_DATA) == gTree_status_OK,
                                        gAkinator_status_TreeErr);

            GAKINATOR_ASSERT_LOG(gTree_addChild(&akinator->tree, nodeId, &yChildId, FAKE_DATA) == gTree_status_OK,
                                        gAkinator_status_TreeErr);

            gTree_Node *nChild = GAKINATOR_NODE_BY_ID(nChildId);
            gTree_Node *yChild = GAKINATOR_NODE_BY_ID(yChildId);
            node = GAKINATOR_NODE_BY_ID(nodeId);
            yChild->data.mode = gAkinator_Node_mode_answer;
            nChild->data.mode = gAkinator_Node_mode_answer;
            if (strIsYes(answer)) {
                strcpy(yChild->data.answer, name);             
                strcpy(nChild->data.answer, node->data.answer);
            } else if (strIsNo(answer)){
                strcpy(nChild->data.answer, name);             
                strcpy(yChild->data.answer, node->data.answer);
            } else {
                assert(!"This should never happen, because wrong answer input is handled beforehead!");
            }
            node->data.mode = gAkinator_Node_mode_question;
            strcpy(node->data.question, question);
        } else if (node->data.mode == gAkinator_Node_mode_none || node->data.mode == gAkinator_Node_mode_unknown) {
            node->data.mode = gAkinator_Node_mode_answer;
            strcpy(node->data.answer, name);
        } else if (node->data.mode == gAkinator_Node_mode_question) {
            fprintf(stderr, "WARNING: the feature has not been implemented yet!\n");
            return gAkinator_status_OK;     
        }
    } else {
        size_t nChildId = parent->child;
        gTree_Node *nChild = GAKINATOR_NODE_BY_ID(nChildId);
        size_t yChildId = nChild->sibling;
        if (yChildId == -1) {
            GAKINATOR_ASSERT_LOG(gTree_addChild(&akinator->tree, parentId, &yChildId, FAKE_DATA) == gTree_status_OK,
                                    gAkinator_status_TreeErr);
            
            gTree_Node *yChild = GAKINATOR_NODE_BY_ID(yChildId);
            yChild->data.mode = gAkinator_Node_mode_answer;
            strcpy(yChild->data.answer, name);
        } else {
            nChild->data.mode = gAkinator_Node_mode_answer;
            strcpy(nChild->data.answer, name);
        }
    }

    return gAkinator_status_OK;
}

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
            if (strIsNo(answer)) {
                nodeId = node->child;
            } else if (strIsYes(answer)) {
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
            if (strIsNo(answer)) {
                fprintf(out, "We don't know the character, what a shame! But you can add your character right here\n");                
                gAkinator_addNew(akinator, nodeId, node);
                goto finish;
            } else if (strIsYes(answer)) {
                fprintf(out, "Great!\nIf you've enjoyed the game, you can by me a coffee\n");        //TODO add BTC wallet addres
                goto finish;
            } else {
                fprintf(out, "Unknown option, please try again\n");
            }
        } else {
            fprintf(out, "We don't know the character, what a shame! But you can add your character right here\n");          
            gAkinator_addNew(akinator, nodeId, node);

            goto finish;
        }
    }
    fprintf(out, "We don't know the character, what a shame! But you can add your character right here\n");                
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

gAkinator_status gAkinator_comp(gAkinator *akinator, size_t oneNodeId, size_t otherNodeId)
{
    GAKINATOR_CHECK_SELF_PTR(akinator);
    GAKINATOR_ASSERT_LOG(  oneNodeId != -1, gAkinator_status_BadId);
    GAKINATOR_ASSERT_LOG(otherNodeId != -1, gAkinator_status_BadId);

    FILE *out = stdout;

    size_t   oneIterId = oneNodeId;
    size_t   oneParentId = -1;
    size_t otherParentId = -1;
    bool   foundFirstMutual = false;

    while (oneIterId != akinator->tree.root) {
        gTree_Node *oneNode = GAKINATOR_NODE_BY_ID(oneIterId);
        size_t otherIterId = otherNodeId;
        while (otherIterId != oneIterId && otherIterId != akinator->tree.root) {
            gTree_Node *otherNode = GAKINATOR_NODE_BY_ID(otherIterId);
            otherParentId = otherIterId;
            otherIterId = otherNode->parent;
        }
        if (otherIterId != akinator->tree.root) {
            if (!foundFirstMutual) {
                fprintf(out, "The difference between %s and %s lies in question:\n%s\n",
                                GAKINATOR_NODE_BY_ID(  oneNodeId)->data.answer,
                                GAKINATOR_NODE_BY_ID(otherNodeId)->data.answer,
                                GAKINATOR_NODE_BY_ID(  oneIterId)->data.question);
                foundFirstMutual = true;
            } else {
                char answer[MAX_LINE_LEN] = "";
                if (GAKINATOR_NODE_BY_ID(oneIterId)->child == oneParentId)
                    strcpy(answer, "NO");
                else 
                    strcpy(answer, "YES");

                fprintf(out, "%s and %s are the same %s for the question:\n%s\n",
                                GAKINATOR_NODE_BY_ID(  oneNodeId)->data.answer,
                                GAKINATOR_NODE_BY_ID(otherNodeId)->data.answer,
                                answer,
                                GAKINATOR_NODE_BY_ID(  oneIterId)->data.question);
            }
        }
        oneParentId = oneIterId;
        oneIterId   = GAKINATOR_NODE_BY_ID(oneIterId)->parent; 
    }

    return gAkinator_status_OK;
}
