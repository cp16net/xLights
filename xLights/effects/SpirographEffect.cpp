#include "SpirographEffect.h"
#include "SpirographPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

SpirographEffect::SpirographEffect(int id) : RenderableEffect(id, "Spirograph")
{
    //ctor
}

SpirographEffect::~SpirographEffect()
{
    //dtor
}
wxPanel *SpirographEffect::CreatePanel(wxWindow *parent) {
    return new SpirographPanel(parent);
}

void SpirographEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    int int_R = SettingsMap.GetInt("SLIDER_Spirograph_R", 0);
    int int_r = SettingsMap.GetInt("SLIDER_Spirograph_r", 0);
    int int_d = SettingsMap.GetInt("SLIDER_Spirograph_d", 0);
    int Animate = SettingsMap.GetInt("TEXTCTRL_Spirograph_Animate", 0);
    int sspeed = SettingsMap.GetInt("TEXTCTRL_Spirograph_Speed", 10);
    int length = SettingsMap.GetInt("TEXTCTRL_Spirograph_Length", 20);
        
    int i,x,y,xc,yc,ColorIdx;
    int mod1440,d_mod;
    float R,r,d,d_orig,t;
    double hyp,x2,y2;
    wxImage::HSVValue hsv,hsv0,hsv1; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    size_t colorcnt=buffer.GetColorCount();
    
    int state = (buffer.curPeriod - buffer.curEffStartPer) * sspeed * buffer.frameTimeInMs / 50;
    double animateState = double((buffer.curPeriod - buffer.curEffStartPer) * Animate * buffer.frameTimeInMs) / 5000.0;
    
    length = length * 18;
    
    xc= (int)(buffer.BufferWi/2); // 20x100 flex strips with 2 fols per strip = 40x50
    yc= (int)(buffer.BufferHt/2);
    R=xc*(int_R/100.0);   //  Radius of the large circle just fits in the width of model
    r=xc*(int_r/100.0); // start little circle at 1/4 of max width
    if(r>R) r=R;
    d=xc*(int_d/100.0);
    
    //  palette.GetHSV(1, hsv1);
    //
    //    A hypotrochoid is a roulette traced by a point attached to a circle of radius r rolling around the inside of a fixed circle of radius R, where the point is a distance d from the center of the interior circle.
    //The parametric equations for a hypotrochoid are:[citation needed]
    //
    //  more info: http://en.wikipedia.org/wiki/Hypotrochoid
    //
    //x(t) = (R-r) * cos t + d*cos ((R-r/r)*t);
    //y(t) = (R-r) * sin t + d*sin ((R-r/r)*t);
    
    mod1440  = state%1440;
    d_orig=d;
    if (Animate) d = d_orig + animateState * d_orig; // should we modify the distance variable each pass through?
    for(i=1; i<=length; i++)
    {
        t = (i+mod1440)*M_PI/180;
        x = (R-r) * cos (t) + d*cos (((R-r)/r)*t) + xc;
        y = (R-r) * sin (t) + d*sin (((R-r)/r)*t) + yc;
        
        if(colorcnt>0) d_mod = (int) buffer.BufferWi/colorcnt;
        else d_mod=1;
        
        x2= pow ((double)(x-xc),2);
        y2= pow ((double)(y-yc),2);
        hyp = (sqrt(x2 + y2)/buffer.BufferWi) * 100.0;
        ColorIdx=(int)(hyp / d_mod); // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
        
        if(ColorIdx>=colorcnt) ColorIdx=colorcnt-1;
        
        buffer.palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
        
        
        buffer.palette.GetHSV(0, hsv0);
        ColorIdx=(state+rand()) % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
        buffer.palette.GetHSV(ColorIdx, hsv1); // Now go and get the hsv value for this ColorIdx
        
        buffer.SetPixel(x,y,hsv);
        //        if(i<=state360) SetPixel(x,y,hsv); // Turn pixel on
        //        else SetPixel(x,y,hsv1);
    }
}