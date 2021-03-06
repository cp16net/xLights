#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include "xLightsMain.h"
#include "heartbeat.h"

#include "SeqSettingsDialog.h"
#include "xLightsXmlFile.h"
#include "BitmapCache.h"

void xLightsFrame::DisplayXlightsFilename(const wxString& filename)
{
    xlightsFilename=filename;
    StaticTextPreviewFileName->SetLabel(filename);
    bool EnableButtons=!filename.IsEmpty();
    bbPlayPause->Enable(EnableButtons);
    bbStop->Enable(EnableButtons);
    StatusBar1->SetStatusText(filename, 1);
}

void xLightsFrame::OnBitmapButtonOpenSeqClick(wxCommandEvent& event)
{
    OpenSequence("");
}

void xLightsFrame::OnButtonNewSequenceClick(wxCommandEvent& event)
{
    NewSequence();
	EnableSequenceControls(true);
}


// load the specified .xseq binary file
void xLightsFrame::SeqLoadXlightsXSEQ(const wxString& filename)
{
    // read xlights file
    wxFileName fn(filename);
    if (fn.GetExt() == "xseq") {
        ReadXlightsFile(filename);
        fn.SetExt("fseq");
    } else {
        ReadFalconFile(filename);
    }
    DisplayXlightsFilename(fn.GetFullPath());
    SeqBaseChannel=1;
    SeqChanCtrlBasic=false;
    SeqChanCtrlColor=false;
}


void xLightsFrame::ResetEffectsXml()
{
    ModelsNode=NULL;
    EffectsNode=NULL;
    PalettesNode=NULL;
    ViewsNode=NULL;
    ModelGroupsNode=NULL;
    SettingsNode=NULL;
    PerspectivesNode = NULL;
}

wxString xLightsFrame::LoadEffectsFileNoCheck()
{
    ResetEffectsXml();
    wxFileName effectsFile;
    effectsFile.AssignDir( CurrentDir );
    effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    wxString myString = "Hello";
    if (!effectsFile.FileExists())
    {
        // file does not exist, so create an empty xml doc
        CreateDefaultEffectsXml();
    }
    else if (!EffectsXml.Load( effectsFile.GetFullPath() ))
    {
        wxMessageBox(_("Unable to load RGB effects file"), _("Error"));
        CreateDefaultEffectsXml();
    }
    wxXmlNode* root=EffectsXml.GetRoot();
    if (root->GetName() != "xrgb")
    {
        wxMessageBox(_("Invalid RGB effects file. Press Save File button to start a new file."), _("Error"));
        CreateDefaultEffectsXml();
    }
    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "models") ModelsNode=e;
        if (e->GetName() == "effects") EffectsNode=e;
        if (e->GetName() == "palettes") PalettesNode=e;
        if (e->GetName() == "views") ViewsNode=e;
        if (e->GetName() == "modelGroups") ModelGroupsNode=e;
        if (e->GetName() == "settings") SettingsNode=e;
        if (e->GetName() == "perspectives") PerspectivesNode=e;
    }
    if (ModelsNode == 0)
    {
        ModelsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "models" );
        root->AddChild( ModelsNode );
    }
    if (EffectsNode == 0)
    {
        EffectsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "effects" );
        EffectsNode->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);
        root->AddChild( EffectsNode );
    }
    if (PalettesNode == 0)
    {
        PalettesNode = new wxXmlNode( wxXML_ELEMENT_NODE, "palettes" );
        root->AddChild( PalettesNode );
    }

    if (ViewsNode == 0)
    {
        ViewsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "views" );
        root->AddChild( ViewsNode );
    }

    if (ModelGroupsNode == 0)
    {
        ModelGroupsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "modelGroups" );
        root->AddChild( ModelGroupsNode );
    }

    if (PerspectivesNode == 0)
    {
        PerspectivesNode = new wxXmlNode( wxXML_ELEMENT_NODE, "perspectives" );
        root->AddChild( PerspectivesNode );
    }

    if(SettingsNode==0)
    {
        SettingsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "settings" );
        root->AddChild( SettingsNode );
        SetXmlSetting("previewWidth","1280");
        SetXmlSetting("previewHeight","720");
    }
    int previewWidth=wxAtoi(GetXmlSetting("previewWidth","1280"));
    int previewHeight=wxAtoi(GetXmlSetting("previewHeight","720"));
    if (previewWidth==0 || previewHeight==0)
    {
        previewWidth = 1280;
        previewHeight = 720;
    }
    SetPreviewSize(previewWidth,previewHeight);

    mBackgroundImage = GetXmlSetting("backgroundImage","");
    if (mBackgroundImage != "" && !wxFileExists(mBackgroundImage)) {
        //image doesn't exist there, lets look for it in show directory and media directory
        wxFileName name(mBackgroundImage);
        name.SetPath(CurrentDir);
        if (!name.Exists()) {
            name.SetPath(mediaDirectory);
        }
        if (name.Exists()) {
            mBackgroundImage = name.GetFullPath();
        }
    }

    mBackgroundBrightness = wxAtoi(GetXmlSetting("backgroundBrightness","100"));
    Slider_BackgroundBrightness->SetValue(mBackgroundBrightness);

    mScaleBackgroundImage = wxAtoi(GetXmlSetting("scaleImage","0"));
    ScaleImageCheckbox->SetValue(mScaleBackgroundImage);

    return effectsFile.GetFullPath();
}

void xLightsFrame::LoadEffectsFile()
{
    wxStopWatch sw; // start a stopwatch timer
    wxString filename=LoadEffectsFileNoCheck();
    // check version, do we need to convert?
    wxString version=EffectsNode->GetAttribute("version", "0000");
    if (version < XLIGHTS_RGBEFFECTS_VERSION)
    {
        // fix tags
        xLightsXmlFile::FixVersionDifferences(filename);

        // load converted file
        LoadEffectsFileNoCheck();

        // fix effect presets
        xLightsXmlFile::FixEffectPresets(EffectsNode);

        // update version
        EffectsNode->DeleteAttribute("version");
        EffectsNode->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);

        UnsavedRgbEffectsChanges = true;
    }

    UpdateModelsList();
    displayElementsPanel->SetSequenceElementsModelsViews(&SeqData, &mSequenceElements,ModelsNode, ModelGroupsNode, ViewsNode);
    CheckForAndCreateDefaultPerpective();
    perspectivePanel->SetPerspectives(PerspectivesNode);
    float elapsedTime = sw.Time()/1000.0; //msec => sec
    StatusBar1->SetStatusText(wxString::Format(_("'%s' loaded in %4.3f sec."), filename, elapsedTime));
}

// returns true on success
bool xLightsFrame::SaveEffectsFile()
{
    wxFileName effectsFile;
    effectsFile.AssignDir( CurrentDir );
    effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    if (!EffectsXml.Save( effectsFile.GetFullPath() ))
    {
        wxMessageBox(_("Unable to save RGB effects file"), _("Error"));
        return false;
    }
    UnsavedRgbEffectsChanges=false;
    return true;
}

void xLightsFrame::CreateDefaultEffectsXml()
{
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, "xrgb" );
    EffectsXml.SetRoot( root );
}

wxXmlNode* xLightsFrame::GetModelNode(const wxString& name)
{
    wxXmlNode* e;
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            if (name == e->GetAttribute("name")) return e;
        }
    }
    return NULL;
}
wxXmlNode* xLightsFrame::CreateModelNodeFromGroup(const wxString &name) {
    wxXmlNode* element;
    std::vector<ModelClass*> models;
    wxString modelString;
    for(wxXmlNode* e=ModelGroupsNode->GetChildren(); e!=NULL; e=e->GetNext() ) {
        if (e->GetName() == "modelGroup") {
            if (name == e->GetAttribute("name")) {
                element = e;
                modelString = e->GetAttribute("models");
                wxArrayString modelNames = wxSplit(modelString, ',');
                for (int x = 0; x < modelNames.size(); x++) {
                    ModelClass *c = GetModelClass(modelNames[x]);
                    if (c != nullptr) {
                        models.push_back(c);
                    }
                }
            }
        }
    }
    if (models.size() == 0) {
        return NULL;
    }
    wxXmlNode * ret = BuildWholeHouseModel(name, element, models);
    ret->AddAttribute("models", modelString);
    return ret;
}
void xLightsFrame::ShowModelsDialog()
{
    ModelListDialog dialog(this);
    wxString name;
    wxXmlNode* e;
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            name=e->GetAttribute("name");
            if (!name.IsEmpty())
            {
                dialog.ListBox1->Append(name,e);
            }
        }
    }
    dialog.HtmlEasyPrint=HtmlEasyPrint;
    dialog.SetSequenceElements(&mSequenceElements);
    dialog.SetNetInfo(&NetInfo);
    dialog.SetModelGroupsNode(ModelGroupsNode);
    dialog.ShowModal();

    // append any new models to the main xml structure
    for(size_t i=0; i<dialog.ListBox1->GetCount(); i++)
    {
        e=(wxXmlNode*)dialog.ListBox1->GetClientData(i);
        if (!e->GetParent())
        {
            ModelsNode->AddChild(e);
        }
    }
    UnsavedRgbEffectsChanges=true;
    UpdateModelsList();
    EnableSequenceControls(true);
}

void xLightsFrame::RenameModelInViews(const wxString& old_name, const wxString& new_name)
{
    // renames view in the rgbeffects xml node
    for(wxXmlNode* view=ViewsNode->GetChildren(); view!=NULL; view=view->GetNext() )
    {
        wxString view_models = view->GetAttribute("models");
        wxArrayString all_models = wxSplit(view_models, ',');
        for( int model = 0; model < all_models.size(); model++ )
        {
            if( all_models[model] == old_name )
            {
                all_models[model] = new_name;
            }
        }
        view_models = wxJoin(all_models, ',');
        view->DeleteAttribute("models");
        view->AddAttribute("models", view_models);
    }
}


void xLightsFrame::SetChoicebook(wxChoicebook* cb, const wxString& PageName)
{
    for(size_t i=0; i<cb->GetPageCount(); i++)
    {
        if (cb->GetPageText(i) == PageName)
        {
            cb->ChangeSelection(i);
            return;
        }
    }
}

void xLightsFrame::OnBitmapButtonSaveSeqClick(wxCommandEvent& event)
{
    SaveSequence();
}

int wxCALLBACK MyCompareFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr WXUNUSED(sortData))
{
	wxString a = ((ModelClass*)item1)->name;
	wxString b = ((ModelClass*)item2)->name;
	return a.CmpNoCase(b);
}


void xLightsFrame::UpdateModelsList()
{
    wxString name, start_channel;
    int end_channel;
    wxArrayString model_names;
    ModelClass *model;
	ListBoxElementList->DeleteAllItems();
    PreviewModels.clear();
    AllModels.clear();
    wxString msg;
    int num_group_models = 0;
	int itemCount = 0;

    // Set models in selected modelgroups as part of display.
    for(wxXmlNode* e=ModelGroupsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "modelGroup")
        {
            if(e->GetAttribute("selected") == "1")
            {
                wxArrayString ModelsInGroup=wxSplit(e->GetAttribute("models"),',');
                for(int i=0;i<ModelsInGroup.size();i++)
                {
                    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
                    {
                        if (e->GetName() == "model")
                        {
                            name=e->GetAttribute("name");
                            if (name == ModelsInGroup[i])
                            {
                                bool model_already_added = false;
                                for(int k = 0; k < model_names.size(); k++)
                                {
                                    if( model_names[k] == name )
                                    {
                                        model_already_added = true;
                                        break;
                                    }
                                }
                                if( !model_already_added )
                                {
                                    model=new ModelClass;
                                    model->SetFromXml(e, NetInfo);

                                    if (model->GetLastChannel() >= NetInfo.GetTotChannels()) {
                                        msg += wxString::Format("%s - last channel: %u\n",name, model->GetLastChannel());
                                    }
                                    if (ModelClass::IsMyDisplay(e))
                                    {
                                        long itemIndex = ListBoxElementList->InsertItem(ListBoxElementList->GetItemCount(),name);
                                        start_channel = e->GetAttribute("StartChannel");
                                        model->SetModelStartChan(start_channel);
                                        wxString string_type = e->GetAttribute("StringType");
                                        int parm1 = wxAtoi(e->GetAttribute("parm1"));
                                        int parm2 = wxAtoi(e->GetAttribute("parm2"));
                                        wxString display_as = e->GetAttribute("DisplayAs");
                                        end_channel = model->GetLastChannel()+1;
                                        ListBoxElementList->SetItem(itemIndex,1,start_channel);
                                        ListBoxElementList->SetItem(itemIndex,2, wxString::Format(wxT("%i"),end_channel));
                                        ListBoxElementList->SetItemPtrData(itemIndex,(wxUIntPtr)model);
                                        PreviewModels.push_back(model);
                                    }
                                    AllModels[name].reset(model);
                                    model_names.push_back(name);
                                    num_group_models++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if( num_group_models == 0 )
    {
        for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            if (e->GetName() == "model")
            {
                name=e->GetAttribute("name");
                if (!name.IsEmpty())
                {
                    model=new ModelClass;
                    model->SetFromXml(e, NetInfo);

                    if (model->GetLastChannel() >= NetInfo.GetTotChannels()) {
                        msg += wxString::Format("%s - last channel: %u\n",name, model->GetLastChannel());
                    }
                    if (ModelClass::IsMyDisplay(e))
                    {
                        long itemIndex = ListBoxElementList->InsertItem(ListBoxElementList->GetItemCount(),name);
                        start_channel = e->GetAttribute("StartChannel");
                        model->SetModelStartChan(start_channel);
                        wxString string_type = e->GetAttribute("StringType");
                        int parm1 = wxAtoi(e->GetAttribute("parm1"));
                        int parm2 = wxAtoi(e->GetAttribute("parm2"));
                        wxString display_as = e->GetAttribute("DisplayAs");
                        end_channel = model->GetLastChannel()+1;
                        ListBoxElementList->SetItem(itemIndex,1,start_channel);
                        ListBoxElementList->SetItem(itemIndex,2, wxString::Format(wxT("%i"),end_channel));
                        ListBoxElementList->SetItemPtrData(itemIndex,(wxUIntPtr)model);
                        PreviewModels.push_back(model);
                    }
                    AllModels[name].reset(model);
                }
            }
        }
    }
    ListBoxElementList->SortItems(MyCompareFunction,0);
    ListBoxElementList->SetColumnWidth(0,wxLIST_AUTOSIZE);
    ListBoxElementList->SetColumnWidth(1,wxLIST_AUTOSIZE);
    ListBoxElementList->SetColumnWidth(2,wxLIST_AUTOSIZE);
    if (msg != "") {
        wxMessageBox(wxString::Format("These models extends beyond the number of configured channels (%u):\n", NetInfo.GetTotChannels()) + msg);
    }
}

void xLightsFrame::SaveSequence()
{
    if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }

    wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
    wxPostEvent(this, playEvent);

    if (xlightsFilename.IsEmpty())
    {
        int saved_text_entry_context = mTextEntryContext;
        mTextEntryContext = TEXT_ENTRY_DIALOG;
        wxString NewFilename;

        wxFileDialog fd(this,
                        "Choose filename to Save Sequence:",
                        CurrentDir,
                        CurrentSeqXmlFile->GetName(),
                        strSequenceSaveAsFileTypes,
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        bool ok = false;
        do
        {
            if (fd.ShowModal() != wxID_OK)
            {
                return;
            }
            // validate inputs
            NewFilename=fd.GetPath();
            NewFilename.Trim();
            ok=true;
            if (NewFilename.IsEmpty())
            {
                ok=false;
                wxMessageBox(_("File name cannot be empty"), _("ERROR"));
            }
        }
        while (!ok);
        wxFileName oName(NewFilename);
        oName.SetExt("fseq");
        DisplayXlightsFilename(oName.GetFullPath());

        oName.SetExt("xml");
        CurrentSeqXmlFile->SetFullName(oName.GetFullName());

        mTextEntryContext = saved_text_entry_context;
    }

    EnableSequenceControls(false);
    wxStopWatch sw; // start a stopwatch timer
    StatusBar1->SetStatusText(_("Saving ")+xlightsFilename);

    CurrentSeqXmlFile->Save(mSequenceElements);
    if (mRenderOnSave) {
        RenderIseqData(true); // render ISEQ layers below the Nutcracker layer
        RenderGridToSeqData();
        RenderIseqData(false);  // render ISEQ layers above the Nutcracker layer
    }
    WriteFalconPiFile(xlightsFilename);
    DisplayXlightsFilename(xlightsFilename);
    float elapsedTime = sw.Time()/1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
    wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"),xlightsFilename,elapsedTime);
    CallAfter(&xLightsFrame::SetStatusText, displayBuff);
    EnableSequenceControls(true);
    mSavedChangeCount = mSequenceElements.GetChangeCount();
}

void xLightsFrame::SaveAsSequence()
{
   if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }
    wxString NewFilename;
    wxFileDialog fd(this,
                    "Choose filename to Save Sequence:",
                    CurrentDir,
                    CurrentSeqXmlFile->GetName(),
                    strSequenceSaveAsFileTypes,
                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    bool ok = false;
    do
    {
        if (fd.ShowModal() != wxID_OK)
        {
            return;
        }
        // validate inputs
        NewFilename=fd.GetPath();
        NewFilename.Trim();
        ok=true;
        if (NewFilename.IsEmpty())
        {
            ok=false;
            wxMessageBox(_("File name cannot be empty"), _("ERROR"));
        }
    }
    while (!ok);
    wxFileName oName(NewFilename);
    oName.SetExt("fseq");
    DisplayXlightsFilename(oName.GetFullPath());

    oName.SetExt("xml");
    CurrentSeqXmlFile->SetFullName(oName.GetFullName());
    SaveSequence();
}

void xLightsFrame::RenderAll()
{
    EnableSequenceControls(false);
    wxStopWatch sw; // start a stopwatch timer
    StatusBar1->SetStatusText(_("Rendering all layers"));
    RenderIseqData(true); // render ISEQ layers below the Nutcracker layer
    RenderGridToSeqData();
    RenderIseqData(false);  // render ISEQ layers above the Nutcracker layer
    float elapsedTime = sw.Time()/1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
    wxString displayBuff = wxString::Format(_("Rendered in %7.3f seconds"),elapsedTime);
    CallAfter(&xLightsFrame::SetStatusText, displayBuff);
    EnableSequenceControls(true);
}

void xLightsFrame::GetModelNames(wxArrayString& a, bool includeGroups)
{
    wxString name;
    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            name=e->GetAttribute("name");
            if (!name.IsEmpty())
            {
                a.Add(name);
            }
        }
    }
    if (includeGroups) {
        for(wxXmlNode* e=ModelGroupsNode->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            if (e->GetName() == "modelGroup")
            {
                name=e->GetAttribute("name");
                if (!name.IsEmpty())
                {
                    a.Add(name);
                }
            }
        }
    }
}


static void enableAllChildControls(wxWindow *parent, bool enable) {
    wxWindowList &ChildList = parent->GetChildren();
    for ( wxWindowList::Node *node = ChildList.GetFirst(); node; node = node->GetNext()) {
        wxWindow *ChildWin = (wxWindow *)node->GetData();
        ChildWin->Enable(enable);
    }
}
static void enableAllToolbarControls(wxAuiToolBar *parent, bool enable) {
    enableAllChildControls((wxWindow *)parent, enable);
    for (int x = 0; x < parent->GetToolCount(); x++) {
        wxAuiToolBarItem * item = parent->FindToolByIndex(x);
        parent->EnableTool(item->GetId(), enable);
    }
    parent->Refresh();
}
static void enableAllMenubarControls(wxMenuBar *parent, bool enable) {
    for (int x = 0; x < parent->GetMenuCount(); x++) {
        wxMenu * menu = parent->GetMenu(x);
        for (int y = 0; y < menu->GetMenuItemCount(); y++) {
            wxMenuItem *item = menu->FindItemByPosition(y);
            menu->Enable(item->GetId(), enable);
        }
    }
    parent->Refresh();
}

void xLightsFrame::EnableSequenceControls(bool enable)
{
    enableAllToolbarControls(MainToolBar, enable);
    enableAllToolbarControls(PlayToolBar, enable && SeqData.NumFrames() > 0);
    enableAllToolbarControls(WindowMgmtToolbar, enable && SeqData.NumFrames() > 0);
    enableAllToolbarControls(EffectsToolBar, enable && SeqData.NumFrames() > 0);
    enableAllToolbarControls(ViewToolBar, enable);
    enableAllToolbarControls(OutputToolBar, enable);

    enableAllChildControls(EffectsPanel1, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(timingPanel, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(perspectivePanel, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(colorPanel, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(effectPalettePanel, enable && SeqData.NumFrames() > 0);

    enableAllMenubarControls(MenuBar, enable);

    if (enable && SeqData.NumFrames() == 0) {
        //no file is loaded, disable save/render buttons
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_SAVE,false);
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_SAVEAS,false);
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_RENDERALL,false);
        Menu_Settings_Sequence->Enable(false);
        MenuItem_File_Save_Sequence->Enable(false);
        MenuItem_File_Close_Sequence->Enable(false);
        MenuSettings->Enable(ID_MENUITEM_RENDER_MODE, false);
    }
    if (!enable && SeqData.NumFrames() > 0) {
        //file is loaded, but we're doing something that requires controls disabled (such as rendering)
        //we need to also disable the quit button
        QuitMenuItem->Enable(false);
    } else {
        QuitMenuItem->Enable(true);
    }
}



//modifed for partially random -DJ
//void djdebug(const char* fmt, ...); //_DJ
wxString xLightsFrame::CreateEffectStringRandom(wxString &settings, wxString &palette)
{
    int eff1;
    if (EffectsPanel1->isRandom_()) { //avoid a few types of random effects
        eff1 = ChooseRandomEffect();
    } else {
        eff1 = EffectsPanel1->EffectChoicebook->GetSelection();
    }

    settings = EffectsPanel1->GetRandomEffectString(eff1);


    palette = colorPanel->GetRandomColorString();
    return EffectNames[eff1];
}

int xLightsFrame::ChooseRandomEffect()
{
    bool BAD_CHOICE=1;
    int eff,count=0;
    int MAX_TRIES=10;

    //    srand (time(NULL));
    while (BAD_CHOICE && count<MAX_TRIES)
    {
        count++;
        eff=rand() % BitmapCache::eff_LASTEFFECT;
        BAD_CHOICE = (BitmapCache::eff_TEXT == eff || BitmapCache::eff_PICTURES == eff || BitmapCache::eff_PIANO == eff
                      || BitmapCache::eff_FACES == eff || BitmapCache::eff_GLEDIATOR == eff
                      || BitmapCache::eff_OFF == eff || BitmapCache::eff_ON == eff);
    }
    if(count==MAX_TRIES) eff=BitmapCache::eff_OFF; // we failed to find a good effect after MAX_TRIES attempts
    return eff;
}



