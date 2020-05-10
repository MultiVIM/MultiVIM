#include <assert.h>

#import "../Views/MVMainWindowView.hh"
#include "../Views/MVTextView.hh"
#import "Lowlevel/MVLog.h"
#import "MVAppPresenter.hh"
#import "MVGridPresenter.hh"
#import "NeoVIMClientThread.h"

MVAppPresenter::MVAppPresenter (NeoVIMClientThread * thread)
    : client (thread), view (800, 600, nil)
{
    _highlightTable =
        (HighlightTableEntry *)calloc (65, sizeof (*_highlightTable));
}

void MVAppPresenter::didUpdateDefaultColoursWithForegroundBackgroundSpecial (
    int fgColour, int bgColour, int spColour)
{
    defaultForegroundColour = fgColour;
    defaultBackgroundColour = bgColour;
    defaultSpecialColour = spColour;
}

void MVAppPresenter::runMainLoop ()
{
    assert (!Fl::lock ());
    view.show ();
    TextView * tView = new TextView (0,
                                     view.mainAreaY () + 24,
                                     view.w (),
                                     view.mainAreaHeight () - 24,
                                     NULL,
                                     "NvimTextView");
    printf ("TextView: %p\n", tView);
    view.addTopLevelTextView (tView);
    printf ("TextView: %p\n", tView);

    gridPresenters[1] = new MVGridPresenter (
        *this, 1, tView, tView->rowCount, tView->columnCount);

    [client attachWithColumns:tView->columnCount rows:tView->rowCount];
    while (1)
    {
        void * msg = 0;
        Fl::lock ();
        Fl::wait ();
        msg = Fl::thread_message ();
        if (msg && (msg == (void *)1))
        {
            [client deliverNotifications];
            // presentEv;
            //[_mainGridPresenter presentEverything];
            //_view->redraw ();
            Fl::flush ();
        }
        Fl::unlock ();
    }
}

int MVAppPresenter::foregroundColourForID (int anID)
{
    HighlightTableEntry * entry;
    int fgc, bgc;

    if (!anID)
        return defaultForegroundColour;

    entry = &_highlightTable[anID];
    fgc = entry->foregroundColour;
    bgc = entry->backgroundColour;

    if (fgc == -1)
        fgc = defaultForegroundColour;
    if (bgc == -1)
        bgc = defaultBackgroundColour;
    if (entry->isReversedVideo)
        return bgc;
    else
        return fgc;
}

int MVAppPresenter::backgroundColourForID (int anID)
{
    HighlightTableEntry * entry;
    int fgc, bgc;

    if (!anID)
        return defaultBackgroundColour;

    entry = &_highlightTable[anID];
    fgc = entry->foregroundColour;
    bgc = entry->backgroundColour;

    if (fgc == -1)
        fgc = defaultForegroundColour;
    if (bgc == -1)
        bgc = defaultBackgroundColour;
    if (entry->isReversedVideo)
        return fgc;
    else
        return bgc;
}

void MVAppPresenter::didUpdate ()
{
    Fl::awake ((void *)1);
}

/* Colours are -1 if default to be used. */
void MVAppPresenter::didUpdateHighlightTableAt (int index, int fgColour,
                                                int bgColour, int spColour,
                                                bool reversed, bool italic,
                                                bool bold, bool struckThrough,
                                                bool underlined,
                                                bool undercurled)

{
    _highlightTable[index] = {.foregroundColour = fgColour,
                              .backgroundColour = bgColour,
                              .specialColour = spColour,
                              .isItalic = italic,
                              .isBold = bold,
                              .isStruckThrough = struckThrough,
                              .isReversedVideo = reversed,
                              .isUnderLined = underlined,
                              .isUnderCurled = undercurled};
}

void MVAppPresenter::didUpdateStatusMessageToKindWithContentShouldReplace (
    OFString * kind, OFArray /* hl_attr, text */ * content, bool replace)
{
    view.msgBar->label ([content[0] UTF8String]);
}

void MVAppPresenter::gridCursorDidGoToRowColumn (int grid, int row, int column)
{
    gridPresenters[grid]->cursorDidGoToRowColumn (row, column);
}

void MVAppPresenter::gridDidUpdateRowFromColumnWithCells (int grid, int row,
                                                          int column,
                                                          OFArray * cells)
{
    gridPresenters[grid]->modelDidUpdateRowFromColumnWithCells (
        row, column, cells);
}

void MVAppPresenter::gridDidResizeToRowsColumns (int grid, int rows, int cols)
{
    gridPresenters[grid]->modelDidResizeToRowsColumns (rows, cols);
}

void MVAppPresenter::gridDidScrollTopBottomLeftRightRowsColumns (
    int grid, int top, int bottom, int left, int right, int rows, int cols)
{
    gridPresenters[grid]->modelDidScrollTopBottomLeftRightRowsColumns (
        top, bottom, left, right, rows, cols);
}

void MVAppPresenter::gridDidInputKey (MVGridPresenter * grid, std::string key)
{
    NeoVIMClientCompletion * comp;
    MVTrace (@"App Presenter: Sending key {%s}\n", key.c_str ());
    comp =
        [client sendRequestWithMethod:@"nvim_input"
                            arguments:@[ [OFString
                                          stringWithUTF8String:key.c_str ()] ]];
}

void MVAppPresenter::gridRequestResizeOfGridToRowsColumns (int grid, int rows,
                                                           int cols)
{
    [client sendRequestWithMethod:@"nvim_ui_try_resize_grid"
                        arguments:@[ @(1), @(cols), @(rows) ]];
}