#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#import <ObjFW/ObjFW.h>

#import "NeoVIMClientThread.h"
#import "Presenters/MVAppPresenter.hh"

int guiMain ();

static struct option options[] = {
    {"neovim-executable", optional_argument, NULL, 'n'}, {NULL, 0, NULL, 0}};

@interface MultiVIM : OFObject <OFApplicationDelegate>
{
    NeoVIMClientThread * _clientThread;
    MVAppPresenter * _appPresenter;
}
@end

// OF_APPLICATION_DELEGATE (MultiVIM)

@implementation MultiVIM

- (void)applicationDidFinishLaunching
{
    int * pArgc;
    char *** pArgv;
    int c;

    _clientThread = [NeoVIMClientThread thread];
    [[OFApplication sharedApplication] getArgumentCount:&pArgc
                                      andArgumentValues:&pArgv];

    while ((c = getopt_long (*pArgc, *pArgv, "n:", options, NULL)) >= 0)
    {
        switch (c)
        {
        case 'n':
            [_clientThread setNeoVimEXE:[OFString stringWithUTF8String:optarg]];
            break;
        default:
            printf ("Invalid Option\n");
        }
    }

    /*     Fl_Double_Window * window = new Fl_Double_Window (640, 480);
        MVMDIWorkspace scroll (0, 0, 640, 480);
        MVMDIWindow box (20, 20, 260, 160, "Win1");
        box.begin ();
        Listener box2 (60, 60, 200, 100, "First");
        box.end ();
        MVMDIWindow box3 (40, 40, 260, 160, "Win2");
        box3.begin ();
        MVMDIWindow box4 (60, 80, 200, 100, "Nested");
        box3.end ();
        // box.add (box2);
        // box3.add (box4);
        scroll.addWindow (box);
        scroll.addWindow (box3);
        window->resizable (scroll);
        window->end ();
        window->show ();
        return Fl::run ();*/

    _appPresenter = new MVAppPresenter (
        _clientThread); //[[MVAppPresenter alloc]
                        // initWithNeoVimClient:_clientThread];
    [_clientThread setDelegate:_appPresenter];
    [_clientThread start];
    //[_appPresenter runMainLoop];
    _appPresenter->runMainLoop ();
    [_clientThread join];
    _clientThread = nil;

    [OFApplication terminate];
}

@end
