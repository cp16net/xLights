#ifndef FIREPANEL_H
#define FIREPANEL_H

//(*Headers(FirePanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../ValueCurveButton.h"

class FirePanel: public wxPanel
{
    void ValidateWindow();
	public:

		FirePanel(wxWindow* parent);
		virtual ~FirePanel();

		//(*Declarations(FirePanel)
		wxBitmapButton* BitmapButton_FireWidth;
		wxStaticText* StaticText83;
		ValueCurveButton* BitmapButton_Fire_HeightVC;
		wxSlider* Slider_Fire_HueShift;
		wxTextCtrl* TextCtrl_Fire_Height;
		wxCheckBox* CheckBox_Fire_GrowWithMusic;
		wxBitmapButton* BitmapButton_FireGrow;
		ValueCurveButton* BitmapButton_Fire_GrowthCyclesVC;
		wxSlider* Slider_Fire_Height;
		wxSlider* Slider_Fire_GrowthCycles;
		wxStaticText* StaticText132;
		wxCheckBox* CheckBox_Fire_Candle;
		wxTextCtrl* TextCtrl_Fire_GrowthCycles;
		wxBitmapButton* BitmapButton_Fire_Candle;
		ValueCurveButton* BitmapButton_Fire_HueShiftVC;
		wxBitmapButton* BitmapButton_Fire_GrowWithMusic;
		wxChoice* Choice_Fire_Location;
		wxBitmapButton* BitmapButton_FireHeight;
		//*)

	protected:

		//(*Identifiers(FirePanel)
		static const long ID_STATICTEXT84;
		static const long ID_SLIDER_Fire_Height;
		static const long ID_VALUECURVE_Fire_Height;
		static const long IDD_TEXTCTRL_Fire_Height;
		static const long ID_BITMAPBUTTON_SLIDER_Fire_Height;
		static const long ID_STATICTEXT133;
		static const long ID_SLIDER_Fire_HueShift;
		static const long ID_VALUECURVE_Fire_HueShift;
		static const long IDD_TEXTCTRL_Fire_HueShift;
		static const long ID_BITMAPBUTTON_SLIDER_Fire_HueShift;
		static const long IDD_SLIDER_Fire_GrowthCycles;
		static const long ID_VALUECURVE_Fire_GrowthCycles;
		static const long ID_TEXTCTRL_Fire_GrowthCycles;
		static const long ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire;
		static const long ID_CHECKBOX_Fire_GrowWithMusic;
		static const long ID_BITMAPBUTTON_Fire_GrowWithMusic;
		static const long ID_CHECKBOX1;
		static const long ID_BITMAPBUTTON1;
		static const long ID_CHOICE_Fire_Location;
		//*)

	public:

		//(*Handlers(FirePanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnCheckBox_Fire_GrowWithMusicClick(wxCommandEvent& event);
        void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
        void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
