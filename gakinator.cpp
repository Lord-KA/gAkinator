#undef EXTRA_VERBOSE

#include "gakinator.h"
#include <cstdlib>


gAkinator_status gAkinator_interface_play(gAkinator *akinator) 
{
    return gAkinator_game(akinator);
}

gAkinator_status gAkinator_interface_comp(gAkinator *akinator) 
{
    size_t   oneNodeId = -1;
    size_t otherNodeId = -1;
    printf("Please provide two node ids:\n");
    if (scanf("%lu %lu", &oneNodeId, &otherNodeId) != 2) {
        fprintf(stdout, "Bad command provided, please try again!\n");
    }
    return gAkinator_comp(akinator, oneNodeId, otherNodeId);
}

gAkinator_status gAkinator_interface_defn(gAkinator *akinator) 
{
    size_t nodeId = -1;
    printf("Please provide nodeId:\n");
    if (scanf("%lu", &nodeId) != 1) {
        fprintf(stdout, "Bad command provided, please try again!\n");
    }
    return gAkinator_definition(akinator, nodeId);
}

gAkinator_status gAkinator_interface_show(gAkinator *akinator) 
{
    FILE *out = fopen("graph.gv", "w");
    gAkinator_status status = gAkinator_dump(akinator, out);
    GAKINATOR_ASSERT_LOG(status == gAkinator_status_OK, status);
    fclose(out);
    system("dot -Tpdf -ograph.pdf ./graph.gv && zathura ./graph.pdf 2> /dev/null");
    return gAkinator_status_OK;
}

gAkinator_status gAkinator_interface_quit(gAkinator *akinator) 
{
    return gAkinator_status_OK;
}

gAkinator_status gAkinator_interface_help(gAkinator *akinator);

struct Command {
    const char *name;
    const char *descr;
    gAkinator_status (*func)(gAkinator *akinator);
};

static const Command cmds[] {
    Command{.name = "play   ", .descr = "play the guessing game",  .func = &gAkinator_interface_play},
    Command{.name = "compare", .descr = "comapare two characters", .func = &gAkinator_interface_comp},
    Command{.name = "define ", .descr = "print the definition of a character", .func = &gAkinator_interface_defn},
    Command{.name = "show   ", .descr = "show the whole graph of questions and answers", .func = &gAkinator_interface_show},
    Command{.name = "quit   ", .descr = "exit the program",        .func = &gAkinator_interface_quit},
    Command{.name = "help   ", .descr = "show this help",          .func = &gAkinator_interface_help},
};

gAkinator_status gAkinator_interface_help(gAkinator *akinator) 
{
    printf("commands:\n");
    for (size_t i = 0; i < sizeof(cmds) / sizeof(cmds[0]); ++i) 
        printf("%d.  %s - %s\n", i + 1, cmds[i].name, cmds[i].descr);

    return gAkinator_status_OK;
}


int main()
{
    gAkinator akinatorStruct;
    gAkinator *akinator = &akinatorStruct;
    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "EXTRA_VERBOSE is defined!!!!\n");
    #endif
    FILE *in = fopen("../examples/not-that-simple.gt", "r");
    gAkinator_ctor(akinator, stderr, in);
    fclose(in);

    fprintf(stdout, "==============================\n");
    fprintf(stdout, "== Akinator game by Lord_KA ==\n");
    fprintf(stdout, "==============================\n");

    gAkinator_interface_help(akinator);

    char command[MAX_LINE_LEN] = "";
    while (!strIsQuit(command)) {
        getline(command, MAX_LINE_LEN, stdin);
        bool foundCommand = false;
        for (size_t i = 0; i < sizeof(cmds) / sizeof(cmds[0]); ++i) {
            if (strIsComb(command, cmds[i].name)) {
                foundCommand = true;
                if ((*cmds[i].func)(akinator) != gAkinator_status_OK) {
                    fprintf(stderr, "Fatal error during execution\n");
                    exit(1);
                }
            }
        }
        if (!foundCommand) 
            fprintf(stdout, "Bad command provided, please try again!\n");
    }

    gAkinator_dtor(akinator);
}
