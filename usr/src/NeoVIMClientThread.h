#import <ObjFW/ObjFW.h>

@interface NeoVIMClientCompletion<Type> : OFObject
{
    Type _result;
    OFCondition * _responseReceived;
}

@property (readonly) Type result;

- (void)completeWithResult:(Type)result;

@end

class MVNVimClientDelegate
{
  public:
    /*
     * This is called from the NeoVIM Client Thread's runloop. It should wake up
     * the UI thread's FLTK runloop, whereupon it can call in its own thread
     * NeoVIMClientThread>>deliverNotifications.
     */
    virtual void didUpdate () = 0;

    virtual void didUpdateDefaultColoursWithForegroundBackgroundSpecial (
        int fgColour, int bgColour, int spColour) = 0;

    /* Colours are -1 if default to be used. */
    virtual void didUpdateHighlightTableAt (int index, int fgColour,
                                            int bgColour, int spColour,
                                            bool reversed, bool italic,
                                            bool bold, bool struckThrough,
                                            bool underlined,
                                            bool undercurled) = 0;

    virtual void didUpdateStatusMessageToKindWithContentShouldReplace (
        OFString * kind, OFArray /* hl_attr, text */ * content,
        bool replace) = 0;

    virtual void gridCursorDidGoToRowColumn (int grid, int row, int column) = 0;

    virtual void gridDidUpdateRowFromColumnWithCells (int grid, int row,
                                                      int column,
                                                      OFArray * cells) = 0;

    virtual void gridDidResizeToRowsColumns (int grid, int rows, int cols) = 0;
    virtual void
    gridDidScrollTopBottomLeftRightRowsColumns (int grid, int top, int bottom,
                                                int left, int right, int rows,
                                                int cols) = 0;
};

@interface NeoVIMClientThread : OFThread <OFStreamDelegate>
{
    OFProcess * neoVIMProcess;
    MVNVimClientDelegate * _delegate;
    OFMutableData * receivedData;
    OFMutableDictionary<OFNumber *, NeoVIMClientCompletion *> * completions;
    OFMutableArray<OFArray *> * notifications;
}

@property MVNVimClientDelegate * delegate;
@property OFString * neoVimEXE;

- (void)attachWithColumns:(int)cols rows:(int)rows;

/* Delivers notifications to a delegate. Called from
   their own thread. */
- (void)deliverNotifications;

- (NeoVIMClientCompletion *)sendRequestWithMethod:(OFString *)methodName
                                        arguments:(OFArray *)arguments;

@end
