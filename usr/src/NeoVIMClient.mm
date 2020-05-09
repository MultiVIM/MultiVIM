#include <fcntl.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#import <ObjFW/OFException.h>
#import <ObjFW/ObjFW.h>

#import "Lowlevel/MVLog.h"
#import "NeoVIMClientThread.h"
#import "ObjFW Categories/OFMutableArray+MV.h"
#import "ObjFW Categories/OFMutableData+MV.h"

@implementation NeoVIMClientCompletion

- initWithId:(int)anId
{
    self = [super init];
    _responseReceived = [OFCondition condition];
    return self;
}

- (void)completeWithResult:(id)result
{
    _result = result;
    [_responseReceived broadcast];
}

- (id)result
{
    [_responseReceived wait];
    return _result;
}

@end

@interface NeoVIMClientThread (Implementation)

/* A method that you call, and it calls on you methods like didReceiveRedraw:?
 * But WHEN should we try to call it? We should perhaps allow for some kind of
 * delegate to be called when any notification is received? Could it handle
 * everything and wake up our FLTK thread for example? */

- (id)makeSynchronousCallToMethod:(OFString *)methodName
                    withArguments:(OFArray *)arguments;

- (bool)stream:(OFStream *)stream
    didReadIntoBuffer:(void *)buffer
               length:(size_t)length
            exception:(nullable id)exception;

@end

@implementation NeoVIMClientThread

@synthesize delegate = _delegate;

- (NeoVIMClientCompletion *)sendRequestWithMethod:(OFString *)methodName
                                        arguments:(OFArray *)arguments
{
    OFArray * request;
    OFData * requestData;
    int cID = rand ();
    NeoVIMClientCompletion * completion = [[NeoVIMClientCompletion alloc] init];

    request = @[ @0, @(cID), methodName, arguments ];
    requestData = [request messagePackRepresentation];
    @synchronized (self)
    {
        @try
        {
            [neoVIMProcess writeData:requestData];
        }
        @catch (OFException * ex)
        {
            printf ("Exception!\n");
            exit (0);
        }
        [completions setObject:completion forKey:@(cID)];
    }

    return completion;
}

- (void)sendNotificationWithMethod:(OFString *)methodName
                         arguments:(OFArray *)arguments
{
    OFArray * request;
    OFData * requestData;

    request = @[ @2, methodName, arguments ];
    requestData = [request messagePackRepresentation];
    @synchronized (self)
    {
        [neoVIMProcess writeData:requestData];
    }
}

- (id)init
{
    self = [super init];
    receivedData = [OFMutableData dataWithCapacity:8192];
    completions = [[OFMutableDictionary alloc] init];
    notifications = [[OFMutableArray alloc] init];
    _neoVimEXE = @"nvim";
    return self;
}

- (bool)stream:(OFStream *)stream
    didReadIntoBuffer:(void *)buffer
               length:(size_t)length
            exception:(nullable id)exception
{
    id received;
    [receivedData addItems:buffer count:length];

    while ([receivedData count])
    {
        received = [receivedData nextMessagePackValue];

        if ([[received objectAtIndex:0] isEqual:@(1)])
        {
            OFNumber * cID = [received objectAtIndex:1];
            NeoVIMClientCompletion * completion =
                [completions objectForKey:cID];

            [completions removeObjectForKey:cID];
            [completion
                completeWithResult:[received objectAtIndex:3]]; /* 2 is error */
        }
        if ([[received objectAtIndex:0] isEqual:@(2)])
        {
            @synchronized (notifications)
            {
                [notifications addObject:received];
            }
        }
    }

    _delegate->didUpdate ();

    return true;
}

static void deliverRedrawCommand (MVNVimClientDelegate * delegate,
                                  OFArray * command)
{
#define cmdIs(x) [command[0] isEqual:x]
    /*bool ignored = cmdIs (@"busy_stop") || cmdIs (@"busy_start") ||
                   cmdIs (@"flush") || cmdIs (@"mode_info_set") ||
                   cmdIs (@"hl_attr_define") || cmdIs (@"hl_group_set");*/

    // if (!ignored)
    //    MVInfo (@"Received redraw command:\n%@", [command description]);

    if ([command[0] isEqual:@"default_colors_set"])
    {
        delegate->didUpdateDefaultColoursWithForegroundBackgroundSpecial (
            [command[1][0] intValue],
            [command[1][1] intValue],
            [command[1][2] intValue]);
    }
    else if ([command[0] isEqual:@"hl_attr_define"])
    {
        OFEnumerator * e = [command objectEnumerator];
        OFArray * el = [e nextObject];
        while (el = [e nextObject])
        {
            OFNumber * hlId = el[0];
            OFDictionary * colours = el[1];
            OFNumber *nFg = colours[@"foreground"],
                     *nBg = colours[@"background"], *nSp = colours[@"special"];
            int fg = nFg ? [nFg intValue] : -1;
            int bg = nBg ? [nBg intValue] : -1;
            int sp = nSp ? [nSp intValue] : -1;
            delegate->didUpdateHighlightTableAt (
                [el[0] intValue],
                fg,
                bg,
                sp,
                [colours[@"reverse"] boolValue],
                [colours[@"italic"] boolValue],
                [colours[@"bold"] boolValue],
                [colours[@"strikethrough"] boolValue],
                [colours[@"underline"] boolValue],
                [colours[@"undercurl"] boolValue]);
        }
    }
    else if ([command[0] isEqual:@"grid_cursor_goto"])
    {
        delegate->gridCursorDidGoToRowColumn ([command[1][0] intValue],
                                              [command[1][1] intValue],
                                              [command[1][2] intValue]);
    }
    else if ([command[0] isEqual:@"grid_line"])
    {
        OFEnumerator * e = [command objectEnumerator];
        OFArray * el = [e nextObject];
        while (el = [e nextObject])
        {
            delegate->gridDidUpdateRowFromColumnWithCells (
                [el[0] intValue], [el[1] intValue], [el[2] intValue], el[3]);
        }
    }
    else if ([command[0] isEqual:@"grid_resize"])
    {
        delegate->gridDidResizeToRowsColumns ([command[1][0] intValue],
                                              [command[1][2] intValue],
                                              [command[1][1] intValue]);
    }
    else if ([command[0] isEqual:@"grid_scroll"])
    {
        delegate->gridDidScrollTopBottomLeftRightRowsColumns (
            [command[1][0] intValue],
            [command[1][1] intValue],
            [command[1][2] intValue],
            [command[1][3] intValue],
            [command[1][4] intValue],
            [command[1][5] intValue],
            [command[1][6] intValue]);
    }
    else if ([command[0] isEqual:@"msg_show"])
    {
        delegate->didUpdateStatusMessageToKindWithContentShouldReplace (
            command[1][0], command[1][1], [command[1][2] boolValue]);
    }
    else
    {
        //[of_stdout writeLine:[command description]];
    }
}

- (void)deliverNotifications
{
    @synchronized (notifications)
    {
        for (OFArray * note in notifications)
        {
            if ([note[1] isEqual:@"redraw"])
            {
                for (OFArray * command in note[2])
                    deliverRedrawCommand (_delegate, command);
            }
        }
        [notifications removeAllObjects];
    }
}

- (void)attachWithColumns:(int)cols rows:(int)rows
{
    [self sendRequestWithMethod:@"nvim_ui_attach"
                      arguments:@[
                          @(cols),
                          @(rows),
                          @{@"ext_linegrid" : @true, @"ext_multigrid" : @false}
                      ]];
    //[self sendRequestWithMethod:@"nvim_subscribe" arguments:@[ @"Gui" ]];
    MVInfo (@"Attached with grid size %d@%d", rows, cols);
}

- (id)main
{
    void * recvBuf = [self allocMemoryWithSize:8192];

    neoVIMProcess = [OFProcess processWithProgram:@"/usr/bin/env"
                                        arguments:@[ _neoVimEXE, @"--embed" ]];
    [neoVIMProcess setDelegate:self];

    MVInfo (@"Started embedded NeoVIM");

    [neoVIMProcess asyncReadIntoBuffer:recvBuf length:8192];

    [[OFRunLoop currentRunLoop] run];

    return nil;
}

@end
