#include "gakinator.h"

int main()
{
    gAkinator akinatorStruct;
    gAkinator *akinator = &akinatorStruct;
    FILE *in = fopen("../examples/not-that-simple.gt", "r");
    gAkinator_ctor(akinator, stderr, in);
    fclose(in);


    // gAkinator_definition(akinator, 21);
    gAkinator_game(akinator);

    FILE *out = fopen("dump.gv", "w");
    gTree_dumpPoolGraphViz(&akinator->tree, out);
    fclose(out);

    gAkinator_dtor(akinator);
}
