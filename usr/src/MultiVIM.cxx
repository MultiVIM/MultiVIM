
#include <fcntl.h>
#include <gc/gc.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "Compiler/AST/AST.hxx"
#include "Compiler/Compiler.hxx"
#include "ObjectMemory/ObjectMemory.hxx"
#include "VM/Interpreter.hxx"

int guiMain ();

static struct option options[] = {
    {"neovim-executable", optional_argument, NULL, 'n'},
    {"smalltalk-source", optional_argument, NULL, 's'},
    {NULL, 0, NULL, 0}};

int main (int argc, char ** argv)
{
    int * pArgc = &argc;
    char *** pArgv = &argv;
    int c;

    // GC_init ();

    memMgr.setupInitialObjects ();

    // _clientThread = [NeoVIMClientThread thread];
    //  [[OFApplication sharedApplication] getArgumentCount:&pArgc
    //                                    andArgumentValues:&pArgv];

    while ((c = getopt_long (*pArgc, *pArgv, "n:s:", options, NULL)) >= 0)
    {
        switch (c)
        {
        case 's':
        {
            ProgramNode * node = MVST_Parser::parseFile (optarg);
            node->registerNames ();
            node->synth ();
            node->generate ();
            break;
        }
        case 'n':
            // [_clientThread setNeoVimEXE:[OFString
            // stringWithUTF8String:optarg]];
            break;
        default:
            printf ("Invalid Option\n");
        }
    }

    ProcessorOopDesc::coldBootMainProcessor ();

    /*    _appPresenter = new MVAppPresenter (
            _clientThread); //[[MVAppPresenter alloc]
                            // initWithNeoVimClient:_clientThread];
        [_clientThread setDelegate:_appPresenter];
        [_clientThread start];
        //[_appPresenter runMainLoop];
        _appPresenter->runMainLoop ();
        [_clientThread join];
        _clientThread = nil;

        [OFApplication terminate];*/
}
