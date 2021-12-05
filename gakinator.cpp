#undef EXTRA_VERBOSE

#include "gakinator.h"
#include <cstdlib>

void helpMsg() 
{
    fprintf(stdout, "commands:\n");
    fprintf(stdout, "\t1. play              - play the guessing game\n");
    fprintf(stdout, "\t2. compare <id> <id> - comapare two characters\n");
    fprintf(stdout, "\t3. define  <id>      - print the definition of a character\n");
    fprintf(stdout, "\t4. show              - show the whole graph of questions and answers\n");
    fprintf(stdout, "\t5. quit              - exit the program\n");
    fprintf(stdout, "\t6. help              - print this help\n");
}

int main()
{
    gAkinator akinatorStruct;
    gAkinator *akinator = &akinatorStruct;
    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "EXTRA_VERBOSE is defined!!!!\n");
    #endif
    // FILE *in = fopen("../examples/node-adding.gt", "r");
    FILE *in = fopen("../examples/not-that-simple.gt", "r");
    gAkinator_ctor(akinator, stderr, in);
    fclose(in);

    fprintf(stdout, "==============================\n");
    fprintf(stdout, "== Akinator game by Lord_KA ==\n");
    fprintf(stdout, "==============================\n");
    helpMsg();
    char command[MAX_LINE_LEN] = "";
    while (!strIsQuit(command)) {
        getline(command, MAX_LINE_LEN, stdin);
        if (!strSkpCmp(command, "play", 1)) {
            gAkinator_game(akinator);
        } else if (!strnSkpCmp(command, "compare", 7)) {
            size_t   oneNodeId = -1;
            size_t otherNodeId = -1;
            if (sscanf(command, "compare %lu %lu ", &oneNodeId, &otherNodeId) != 2) {
                fprintf(stdout, "Bad command provided, please try again!\n");
                continue;
            }
            gAkinator_comp(akinator, oneNodeId, otherNodeId);
        } else if (!strnSkpCmp(command, "define", 6)) {
            size_t nodeId = -1;
            if (sscanf(command, "define %lu ", &nodeId) != 1) {
                fprintf(stdout, "Bad command provided, please try again!\n");
                continue;
            }
            gAkinator_definition(akinator, nodeId);
        } else if (!strSkpCmp(command, "show", 1)) {
            FILE *out = fopen("graph.gv", "w");
            gTree_dumpPoolGraphViz(&akinator->tree, out);
            fclose(out);
            system("dot -Tpdf -ograph.pdf ./graph.gv && zathura ./graph.pdf 2> /dev/null");
        } else if (!strSkpCmp(command, "help", 1)) {
            helpMsg();
        } else if (!strIsQuit(command)) {
            fprintf(stdout, "Bad command provided, please try again!\n");
            continue;
        }
    }

    gAkinator_dtor(akinator);
}
