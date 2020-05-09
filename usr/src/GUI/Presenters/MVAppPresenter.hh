#import <ObjFW/ObjFW.h>

#include <map>

#import "NeoVIMClientThread.h"
#include "Presenters/MVGridPresenter.hh"
#include "Views/MVMainWindowView.hh"

/*@class MVAppModel;
@class MVGridPresenter;

@interface MVAppPresenter : OFObject
{
    NeoVIMClientThread * _client;
    MVGridPresenter * _mainGridPresenter;
}

@property (readonly) MVMainWindowView * view;
@property (readonly) MVAppModel * model;

- (instancetype)initWithNeoVimClient:(NeoVIMClientThread *)aClient;
- (void)runMainLoop;

- (void)gridWantsResize:(MVGridPresenter *)grid
                 toRows:(int)rows
                columns:(int)columns;
- (void)grid:(MVGridPresenter *)grid didInputKey:(OFString *)key;

- (void)presentStatusMessage;
@end*/

class MVAppPresenter : public MVNVimClientDelegate
{
    struct HighlightTableEntry
    {
        /* If any of these three is `-1`, then the default is to be used
         * instead. */
        int foregroundColour, backgroundColour, specialColour;

        bool isItalic, isBold, isStruckThrough;
        bool isReversedVideo;
        /* The underline or undercurl are coloured with specialColour. */
        bool isUnderLined, isUnderCurled;
    };

    NeoVIMClientThread * client;
    MVMainWindowView view;
    HighlightTableEntry * _highlightTable;

    int defaultForegroundColour, defaultBackgroundColour, defaultSpecialColour;

    std::map<int, MVGridPresenter *> gridPresenters;

  public:
    MVAppPresenter (NeoVIMClientThread * thread);

    void runMainLoop ();

    int foregroundColourForID (int anID);
    int backgroundColourForID (int anID);

    /* Model delegates */
    virtual void didUpdate ();

    virtual void didUpdateDefaultColoursWithForegroundBackgroundSpecial (
        int fgColour, int bgColour, int spColour);

    /* Colours are -1 if default to be used. */
    virtual void didUpdateHighlightTableAt (int index, int fgColour,
                                            int bgColour, int spColour,
                                            bool reversed, bool italic,
                                            bool bold, bool struckThrough,
                                            bool underlined, bool undercurled);

    virtual void didUpdateStatusMessageToKindWithContentShouldReplace (
        OFString * kind, OFArray /* hl_attr, text */ * content, bool replace);

    virtual void gridCursorDidGoToRowColumn (int grid, int row, int column);

    virtual void gridDidUpdateRowFromColumnWithCells (int grid, int row,
                                                      int column,
                                                      OFArray * cells);

    virtual void gridDidResizeToRowsColumns (int grid, int rows, int cols);

    virtual void gridDidScrollTopBottomLeftRightRowsColumns (
        int grid, int top, int bottom, int left, int right, int rows, int cols);

    /* GridPresenter requests */
    void gridRequestResizeOfGridToRowsColumns (int grid, int rows, int cols);
    void gridDidInputKey (MVGridPresenter * grid, std::string key);
};