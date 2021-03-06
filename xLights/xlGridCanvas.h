#ifndef XLGRIDCANVAS_H
#define XLGRIDCANVAS_H

#include "wx/wx.h"
#include "xlGLCanvas.h"
#include "Effect.h"
#include "Image.h"
#include "XlightsDrawable.h"

class ModelClass;  // forward declaration

class xlGridCanvas : public xlGLCanvas
{
    public:

        xlGridCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~xlGridCanvas();

        virtual void SetEffect(Effect* effect_) = 0;
        Effect* GetEffect() {return mEffect;}
        void SetModelClass(ModelClass* cls) {mModelClass = cls;}
        void SetNumColumns(int columns) {mColumns = columns;}
        void SetNumRows(int rows) {mRows = rows;}
        int GetCellSize() {return mCellSize;}
        void AdjustSize(wxSize& parent_size);
        virtual void ForceRefresh() = 0;

    protected:
        virtual void InitializeGLCanvas() = 0;

        void DrawBaseGrid();
        void DrawEffect();
        int GetRowCenter(int percent);
        int GetColumnCenter(int percent);
        int SetRowCenter(int position);
        int SetColumnCenter(int position);
        int GetCellFromPosition(int position);
        int calcCellFromPercent(int value, int base);
        int calcPercentFromCell(int value, int base);

        Effect* mEffect;
        ModelClass* mModelClass;
        xlColor* mGridlineColor;
        int mCellSize;
        int mColumns;
        int mRows;
        bool mDragging;

        DECLARE_EVENT_TABLE()
};

#endif // XLGRIDCANVAS_H
