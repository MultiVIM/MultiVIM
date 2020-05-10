#include <assert.h>

#include "../Views/MVMainWindowView.hh"
#include "../Views/MVTextView.hh"
#include "Lowlevel/MVLog.h"
#include "MVAppPresenter.hh"
#import "MVGridPresenter.hh"

MVGridPresenter::MVGridPresenter (MVAppPresenter & appPresenter, int gridNumber,
                                  TextView * view, int rows, int cols)
    : appPresenter (appPresenter), gridNumber (gridNumber), rowCount (rows),
      columnCount (cols), cursorRow (0), cursorColumn (0), view (view)
{
    view->presenter = this;
    _rows = NULL;
    rowCount = 0;
    modelDidResizeToRowsColumns (rows, cols);
}

void MVGridPresenter::modelDidUpdateRowFromColumnWithCells (int row, int col,
                                                            OFArray * cells)
{
    int hlID;
    for (OFArray * cell in cells)
    {
        int repetitions = 1;
        if ([cell count] == 3)
        {
            repetitions = [cell[2] intValue];
        }
        if ([cell count] >= 2)
        {
            hlID = [cell[1] intValue];
        }
        while (repetitions--)
        {
            _rows[row][col].hlID = hlID;
            _rows[row][col++].text = strdup ([cell[0] UTF8String]);
        }
    }
    presentAllRows ();
}

void MVGridPresenter::modelDidResizeToRowsColumns (int newRowCount,
                                                   int newColumnCount)
{
    for (int r = 0; r < rowCount; r++)
    {
        for (int c = 0; c < columnCount; c++)
        {
            free (_rows[r][c].text);
        }
        free (_rows[r]);
    }
    free (_rows);

    rowCount = newRowCount, columnCount = newColumnCount;
    _rows = (MVGridCell **)calloc (newRowCount, sizeof (*_rows));

    for (int r = 0; r < newRowCount; r++)
    {
        _rows[r] = (MVGridCell *)calloc (newColumnCount, sizeof (**_rows));
    }
}

void MVGridPresenter::cursorDidGoToRowColumn (int row, int col)
{
    view->moveCursorToRowColumn (row, col);
    view->redraw ();
}

void MVGridPresenter::presentAllRows ()
{
    int rowsMax = MIN (rowCount, view->rowCount);
    int colsMax = MIN (columnCount, view->columnCount);
    for (int i = 0; i < rowsMax; i++)
    {
        int fgColour, bgColour;
        int hlAttr = -1;
        std::string string;

        view->clearRowText (i);

#define AddRow()                                                               \
    bgColour = appPresenter.backgroundColourForID (hlAttr);                    \
    fgColour = appPresenter.foregroundColourForID (hlAttr);                    \
    view->addRowText (i, string.c_str (), fgColour, bgColour)

        // printf ("I: %d, rows max: %d\n", i,  count]);
        for (int c = 0; c < colsMax; c++)
        {
            MVGridCell * cell = &_rows[i][c];
            if (hlAttr != cell->hlID && string.length ())
            {
                AddRow ();
                string = "";
            }
            hlAttr = cell->hlID;
            if (cell->text)
                string.append (cell->text);
            else
                string.append (" ");
        }
        AddRow ();
        view->renderRowText (i);
    }
    view->redraw ();
}

void MVGridPresenter::modelDidScrollTopBottomLeftRightRowsColumns (
    int top, int bottom, int left, int right, int nRows, int nCols)
{
    int toTop = top - nRows;
    int toBottom = bottom - nRows;
    int start = (nRows >= 0) ? top : bottom - 1;
    int end = (nRows >= 0) ? bottom : top;

    for (int fromRow = (nRows >= 0) ? top : bottom - 1;
         (nRows >= 0) ? (fromRow < end) : (fromRow >= end);
         (nRows >= 0) ? fromRow++ : fromRow--)
    {
        int toRow = fromRow - nRows;
        if (top <= toRow && toRow < bottom)
        {
            for (int idx = left; idx < right; idx++)
            {
                _rows[toRow][idx + nCols] = _rows[fromRow][idx];
            }
        }
    }

    view->redraw ();
}

void MVGridPresenter::viewDidResizeToRowsColumns (int rows, int cols)
{
    appPresenter.gridRequestResizeOfGridToRowsColumns (gridNumber, rows, cols);
}

void MVGridPresenter::viewDidInputKey (std::string key)
{
    appPresenter.gridDidInputKey (this, key);
}