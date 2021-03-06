#ifndef MODELDIALOG_H
#define MODELDIALOG_H

#include <wx/xml/xml.h>
#include <map>

//(*Headers(ModelDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/radiobut.h>
#include <wx/slider.h>
#include <wx/grid.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/clrpicker.h>
//*)

class NetInfoClass;

class ModelDialog: public wxDialog
{
public:

    ModelDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
    virtual ~ModelDialog();
    void UpdateLabels();
    void SetCustomGridData(const wxString& customChannelData);
    wxString GetCustomGridData();
    void SetFromXml(wxXmlNode* e, NetInfoClass *netInfo, const wxString& NameSuffix=wxEmptyString);
    void UpdateXml(wxXmlNode* e);

    //(*Declarations(ModelDialog)
    wxStaticText* StaticText9;
    wxSpinCtrl* SpinCtrl_parm2;
    wxColourPickerCtrl* ColorPicker;
    wxSpinCtrl* SpinCtrl_parm1;
    wxStaticText* StaticText_Strings;
    wxTextCtrl* TextCtrl_Name;
    wxStaticText* StaticTextCustomModel;
    wxStaticText* StaticText2;
    wxStaticText* StaticText6;
    wxSpinCtrl* SpinCtrlModelBrightness;
    wxSpinCtrl* SpinCtrl_parm3;
    wxFlexGridSizer* OuterSizer;
    wxStaticText* StaticText8;
    wxStaticText* StaticText1;
    wxTextCtrl* StarSizes;
    wxStaticText* StaticText3;
    wxChoice* Choice_StringType;
    wxSpinCtrl* TreeFirstStringForExport;
    wxButton* Button_CustomModelZoomIn;
    wxGrid* GridCustom;
    wxBitmapButton* BitmapButtonCustomPaste;
    wxFlexGridSizer* LeftGridSizer;
    wxRadioButton* RadioButton_TopLeft;
    wxRadioButton* RadioButton_BotLeft;
    wxCheckBox* CheckBox_MyDisplay;
    wxStaticText* StaticText5;
    wxStaticText* StaticText7;
    wxBitmapButton* BitmapButtonCustomCopy;
    wxCheckBox* cbIndividualStartNumbers;
    wxStaticText* ExtraParameterLabel;
    wxSlider* Slider_Model_Brightness;
    wxButton* ButtonCustomModelHelp;
    wxGrid* gridStartChannels;
    wxBitmapButton* BitmapButtonCustomCut;
    wxChoice* Choice_DisplayAs;
    wxStaticText* StaticText4;
    wxButton* Button_CustomModelZoomOut;
    wxRadioButton* RadioButton_BotRight;
    wxSpinCtrl* SpinCtrl_StartChannel;
    wxRadioButton* RadioButton_TopRight;
    wxStaticText* StaticText_Strands;
    wxStaticText* StaticText_Nodes;
    wxSpinCtrl* OutputSpinCtrl;
    //*)

protected:

    //(*Identifiers(ModelDialog)
    static const long ID_STATICTEXT1;
    static const long ID_TEXTCTRL1;
    static const long ID_STATICTEXT5;
    static const long ID_CHOICE_DisplayAs;
    static const long ID_STATICTEXT12;
    static const long ID_CHOICE_STRING_TYPE;
    static const long ID_COLOURPICKERCTRL1;
    static const long ID_STATICTEXT2;
    static const long ID_SPINCTRL1;
    static const long ID_STATICTEXT3;
    static const long ID_SPINCTRL2;
    static const long ID_STATICTEXT4;
    static const long ID_SPINCTRL3;
    static const long ID_STATICTEXT6;
    static const long ID_SPINCTRL4;
    static const long ID_STATICTEXT9;
    static const long ID_SPINCTRL6;
    static const long ID_STATICTEXT8;
    static const long ID_RADIOBUTTON1;
    static const long ID_RADIOBUTTON2;
    static const long ID_STATICTEXT11;
    static const long ID_RADIOBUTTON4;
    static const long ID_RADIOBUTTON3;
    static const long ID_STATICTEXT10;
    static const long ID_CHECKBOX1;
    static const long ID_STATICTEXT7;
    static const long ID_Slider_Model_Brightness;
    static const long ID_SPINCTRLMODELBRIGHTNESS;
    static const long ID_STATICTEXT15;
    static const long ID_SPINCTRL5;
    static const long ID_TEXTCTRL3;
    static const long ID_BUTTON3;
    static const long ID_BUTTON4;
    static const long ID_BUTTON1;
    static const long ID_CHECKBOX2;
    static const long ID_GRID_START_CHANNELS;
    static const long ID_STATICTEXT14;
    static const long ID_BITMAPBUTTON_CUSTOM_CUT;
    static const long ID_BITMAPBUTTON_CUSTOM_COPY;
    static const long ID_BITMAPBUTTON_CUSTOM_PASTE;
    static const long ID_BUTTON_CUSTOM_MODEL_HELP;
    static const long ID_BUTTON_CustomModelZoomIn;
    static const long ID_BUTTON_CustomModelZoomOut;
    static const long ID_GRID_Custom;
    //*)

private:

    //(*Handlers(ModelDialog)
    void OnChoice_DisplayAsSelect(wxCommandEvent& event);
    void OncbIndividualStartNumbersClick(wxCommandEvent& event);
    void OnSpinCtrl_parm1Change(wxSpinEvent& event);
    void OnSpinCtrl_parm2Change(wxSpinEvent& event);
    void OnSpinCtrl_StartChannelChange(wxSpinEvent& event);
    void OngridStartChannelsCellChange(wxGridEvent& event);
    void OnButtonCustomModelHelpClick(wxCommandEvent& event);
    void OnChoice_StringTypeSelect(wxCommandEvent& event);
    void OnGridCustomCellChange(wxGridEvent& event);
    void OnBitmapButtonCustomPasteClick(wxCommandEvent& event);
    void OnBitmapButtonCustomCopyClick(wxCommandEvent& event);
    void OnBitmapButtonCustomCutClick(wxCommandEvent& event);
    void OnSpinCtrl_parm3Change(wxSpinEvent& event);
    void OnSlider_Model_BrightnessCmdScroll(wxScrollEvent& event);
    void OnSlider_Model_BrightnessScrollTop(wxScrollEvent& event);
    void OnSlider_Model_BrightnessCmdScroll1(wxScrollEvent& event);
    void OnButton_CustomModelZoomOutClick(wxCommandEvent& event);
    void OnButton_CustomModelZoomInClick(wxCommandEvent& event);
    void OnSpinCtrlModelBrightnessChange(wxSpinEvent& event);
    void OnAppearanceButtonClicked(wxCommandEvent& event);
    void OnNamesButtonClick(wxCommandEvent& event);
    void OnFacesButtonClick(wxCommandEvent& event);
    //*)

    void SetReadOnly(bool);
    void UpdateStartChannels();
    void ResizeCustomGrid();
    bool IsCustom();
    int GetNumberOfStrings();
    wxString StartChanAttrName(int idx);
    int GetChannelsPerStringStd();
    int GetCustomMaxChannel();
    void CutOrCopyToClipboard(bool IsCut);

    bool HasCustomData;

    DECLARE_EVENT_TABLE()
    
    NetInfoClass *netInfo;
    int pixelStyle = 0;
    int pixelSize = 2;
    int transparency = 0;
    int blackTransparency = 0;
    int previewBrightness = 100;
    wxString strandNames;
    wxString nodeNames;
    std::map<wxString, std::map<wxString,wxString> > faceInfo;
};

#endif
