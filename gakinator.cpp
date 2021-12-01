#include "gakinator.h"

int main()
{
    gAkinator akinatorStruct;
    gAkinator *akinator = &akinatorStruct;
    FILE *in = fopen("../examples/not-that-simple.gt", "r");
    gAkinator_ctor(akinator, stderr, in);
    fclose(in);

    gTree_dumpPoolGraphViz(&akinator->tree, stdout);

    gAkinator_definition(akinator, 21);
    // gAkinator_game(akinator);

    gAkinator_dtor(akinator);
}
