#include "gakinator.h"

int main()
{
    gAkinator akinatorStruct;
    gAkinator *akinator = &akinatorStruct;
    FILE *in = fopen("storage.gt", "r");
    gAkinator_ctor(akinator, stderr, in);
    fclose(in);

    gTree_dumpPoolGraphViz(&akinator->tree, stdout);

    gAkinator_dtor(akinator);
}
