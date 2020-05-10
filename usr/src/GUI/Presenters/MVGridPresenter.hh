#import <ObjFW/ObjFW.h>

#include "../Views/MVTextView.hh"

/*@class MVGridModel;
@class MVAppPresenter;

@interface MVGridPresenter : OFObject
{
    MVAppPresenter * _appPresenter;
    TextView * _view;
}

@property (readonly) MVGridModel * model;

- (instancetype)initWithAppPresenter:(MVAppPresenter *)appPresenter;

- (void)viewDidResizeToRows:(int)rows columns:(int)columns;
- (void)viewDidInputKey:(OFString *)key;

- (void)presentEverything;
@end*/

class MVAppPresenter;

struct MVGridCell
{
    int hlID;
    char * text;
};

class MVGridPresenter
{
    MVAppPresenter & appPresenter;
    TextView * view;

    int gridNumber;

    MVGridCell ** _rows;
    int rowCount, columnCount;
    int cursorRow, cursorColumn;

  public:
    MVGridPresenter (MVAppPresenter & appPresenter, int gridNumber,
                     TextView * view, int rows, int cols);

    void modelDidUpdateRowFromColumnWithCells (int row, int col,
                                               OFArray * cells);
    void modelDidResizeToRowsColumns (int rows, int cols);
    void cursorDidGoToRowColumn (int row, int col);
    void modelDidScrollTopBottomLeftRightRowsColumns (int top, int bottom,
                                                      int left, int right,
                                                      int rows, int cols);

    void presentAllRows ();

    void viewDidResizeToRowsColumns (int rows, int cols);
    void viewDidInputKey (std::string key);
};