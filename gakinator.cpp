#include "gakinator.h"

int main()
{
    gAkinator akinatorStruct;
    gAkinator *akinator = &akinatorStruct;
    #ifdef EXTRA_VERBOSE
        fpritnf(stderr, "EXTRA_VERBOSE is defined!!!!\n");
    #endif
    // FILE *in = fopen("../examples/node-adding.gt", "r");
    FILE *in = fopen("../examples/not-that-simple.gt", "r");
    gAkinator_ctor(akinator, stderr, in);
    fclose(in);


    // gAkinator_definition(akinator, 21);
    // gAkinator_game(akinator);
    gAkinator_comp(akinator, 5, 4);

    FILE *out = fopen("dump.gv", "w");
    gTree_dumpPoolGraphViz(&akinator->tree, out);
    fclose(out);

    gAkinator_dtor(akinator);
}
