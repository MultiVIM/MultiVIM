#include <getopt.h>

#include "Compiler/AST/AST.hxx"
#include "Compiler/Compiler.hxx"
#include "ObjectMemory/ObjectMemory.hxx"

static struct option options[] = {{"source", optional_argument, NULL, 's'},
                                  {NULL, 0, NULL, 0}};

int main (int argc, char * argv[])
{
    int c;
    memMgr.coldBoot ();

    while ((c = getopt_long (argc, argv, "s:", options, NULL)) >= 0)
    {
        switch (c)
        {
        case 's':
        {
            ProgramNode * node = MVST_Parser::parseFile (optarg);
            node->synth ();
            break;
        }
        default:
            printf ("Invalid Option\n");
        }
    }
}