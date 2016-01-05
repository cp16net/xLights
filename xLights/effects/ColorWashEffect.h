#ifndef COLORWASHEFFECT_H
#define COLORWASHEFFECT_H

#include "RenderableEffect.h"


class ColorWashEffect : public RenderableEffect
{
    public:
        ColorWashEffect(int id);
        virtual ~ColorWashEffect();
    
        virtual void SetDefaultParameters(ModelClass *cls);
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // COLORWASHEFFECT_H