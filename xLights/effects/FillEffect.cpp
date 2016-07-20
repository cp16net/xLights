#include "FillEffect.h"
#include "FillPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


#include "../../include/fill-16.xpm"
#include "../../include/fill-64.xpm"

FillEffect::FillEffect(int i) : RenderableEffect(i, "Fill", fill_16, fill_64, fill_64, fill_64, fill_64)
{
    //ctor
}

FillEffect::~FillEffect()
{
    //dtor
}

wxPanel *FillEffect::CreatePanel(wxWindow *parent) {
    return new FillPanel(parent);
}

static void UpdateFillColor(int &position, int &band_color, int colorcnt, int color_size, int shift)
{
    if( shift == 0 ) return;
    if( shift > 0 )
    {
        int index = 0;
        while( index < shift )
        {
            position++;
            if( position >= color_size )
            {
                band_color++;
                band_color %= colorcnt;
                position = 0;
            }
            index++;
        }
    }
    else
    {
        int index = 0;
        while( index > shift )
        {
            position--;
            if( position < 0 )
            {
                band_color++;
                band_color %= colorcnt;
                position = color_size-1;
            }
            index--;
        }
    }
}


static inline int GetDirection(const std::string & DirectionString) {
    if ("Up" == DirectionString) {
        return 0;
    } else if ("Down" == DirectionString) {
        return 1;
    } else if ("Left" == DirectionString) {
        return 2;
    } else if ("Right" == DirectionString) {
        return 3;
    }
    return 0;
}

void FillEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    int position = GetValueCurveInt("Fill_Position", 100, SettingsMap, eff_pos);
    double pos_pct = (double)position / 100.0;
    int Direction = GetDirection(SettingsMap["CHOICE_Fill_Direction"]);
    int BandSize = GetValueCurveInt("Fill_Band_Size", 0, SettingsMap, eff_pos);
    int SkipSize = GetValueCurveInt("Fill_Skip_Size", 0, SettingsMap, eff_pos);
    int offset = GetValueCurveInt("Fill_Offset", 0, SettingsMap, eff_pos);
    int offset_in_pixels = SettingsMap.GetBool("CHECKBOX_Fill_Offset_In_Pixels", true);
    switch (Direction)
    {
        default:
        case 0:  // Up
        case 1:  // Down
            if( !offset_in_pixels ) {
                offset = ((buffer.BufferHt-1) * offset) / 100;
            } else {
                offset %= buffer.BufferHt;
            }
            break;
        case 2:  // Left
        case 3:  // Right
            if( !offset_in_pixels ) {
                offset = ((buffer.BufferWi-1) * offset) / 100;
            } else {
                offset %= buffer.BufferWi;
            }
            break;

    }


    int x,y;
    xlColor color;
    size_t colorcnt = buffer.GetColorCount();
    int color_size = BandSize +  SkipSize;
    int repeat_size = color_size * colorcnt;
    int band_pos = buffer.curPeriod - buffer.curEffStartPer;
    int current_color = 0;
    int current_pos = 0;

    if( BandSize == 0 ) {
        double color_val = eff_pos * (colorcnt-1);
        int color_int = (int)color_val;
        double color_pct = color_val - (double)color_int;
        int color2 = std::min(color_int+1, (int)colorcnt-1);
        if( color_int < color2 )
        {
            buffer.Get2ColorBlend(color_int, color2, std::min( color_pct, 1.0), color);
        }
        else
        {
            buffer.palette.GetColor(color2, color);
        }
    }

    switch (Direction)
    {
        if( BandSize > 0 ) {
            UpdateFillColor(current_pos, current_color, colorcnt, color_size, 1);
        }
        default:
        case 0:  // Up
            offset %= buffer.BufferHt;
            for( y=offset; y<buffer.BufferHt*pos_pct+offset; y++)
            {
                if( BandSize > 0 ) {
                    color = xlBLACK;
                    if( current_pos < BandSize )
                    {
                        buffer.palette.GetColor(current_color, color);
                    }
                }
                int y_pos = y;
                if( y_pos >= buffer.BufferHt ) y_pos -= buffer.BufferHt;
                for (x=0; x<buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y_pos, color);
                }
                if( BandSize > 0 ) {
                    UpdateFillColor(current_pos, current_color, colorcnt, color_size, 1);
                }
            }
            break;
        case 1:  // Down
            offset %= buffer.BufferHt;
            for( y=buffer.BufferHt-1-offset; y>=buffer.BufferHt*(1.0-pos_pct)-offset; y--)
            {
                if( BandSize > 0 ) {
                    color = xlBLACK;
                    if( current_pos < BandSize )
                    {
                        buffer.palette.GetColor(current_color, color);
                    }
                }
                int y_pos = y;
                if( y_pos < 0 ) y_pos += buffer.BufferHt;
                for (x=0; x<buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y_pos, color);
                }
                if( BandSize > 0 ) {
                    UpdateFillColor(current_pos, current_color, colorcnt, color_size, 1);
                }
            }
            break;
        case 2:  // Left
            offset %= buffer.BufferWi;
            for (x=buffer.BufferWi-1-offset; x>=buffer.BufferWi*(1.0-pos_pct)-offset; x--)
            {
                if( BandSize > 0 ) {
                    color = xlBLACK;
                    if( current_pos < BandSize )
                    {
                        buffer.palette.GetColor(current_color, color);
                    }
                }
                int x_pos = x;
                if( x_pos < 0 ) x_pos += buffer.BufferWi;
                for( y=0; y<buffer.BufferHt; y++)
                {
                    buffer.SetPixel(x_pos, y, color);
                }
                if( BandSize > 0 ) {
                    UpdateFillColor(current_pos, current_color, colorcnt, color_size, 1);
                }
            }
            break;
        case 3:  // Right
            offset %= buffer.BufferWi;
            for (x=offset; x<buffer.BufferWi*pos_pct+offset; x++)
            {
                if( BandSize > 0 ) {
                    color = xlBLACK;
                    if( current_pos < BandSize )
                    {
                        buffer.palette.GetColor(current_color, color);
                    }
                }
                int x_pos = x;
                if( x_pos >= buffer.BufferWi ) x_pos -= buffer.BufferWi;
                for( y=0; y<buffer.BufferHt; y++)
                {
                    buffer.SetPixel(x_pos, y, color);
                }
                if( BandSize > 0 ) {
                    UpdateFillColor(current_pos, current_color, colorcnt, color_size, 1);
                }
            }
            break;

    }
 }
