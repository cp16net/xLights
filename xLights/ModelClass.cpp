/***************************************************************
 * Name:      ModelClass.cpp
 * Purpose:   Represents Display Model
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-10-22
 * Copyright: 2012 by Matt Brown
 * License:
     This file is part of xLights.

    xLights is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xLights is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xLights.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************/
#include "ModelClass.h"
#include <wx/msgdlg.h>
#include <wx/tokenzr.h>
#include <wx/graphics.h>
#include "xLightsMain.h" //for Preview and Other model collections
#include "Color.h"
#include "DrawGLUtils.h"

static inline void TranslatePointDoubles(double radians,double x, double y,double &x1, double &y1) {
    x1 = cos(radians)*x-(sin(radians)*y);
    y1 = sin(radians)*x+(cos(radians)*y);
}

void ModelClass::InitWholeHouse(const wxString &WholeHouseData, bool zeroBased) {
    long xCoord,yCoord,actChn;
    int lastActChn=0;
    wxArrayString data;
    SetBufferSize(parm2,parm1);
    SetRenderSize(parm2,parm1);
    wxString stringType;

    Nodes.clear();
    int minChan = 9999999;
    int maxChan = -1;
    if(WholeHouseData.Length()> 0) {
        wxArrayString wholeHouseDataArr=wxSplit(WholeHouseData,';');
        int coordinateCount=wholeHouseDataArr.size();

        // Load first coordinate
        data=wxSplit(wholeHouseDataArr[0],',');
        data[0].ToLong(&actChn);
        if (actChn > maxChan) {
            maxChan = actChn;
        }
        if (actChn < minChan) {
            minChan = actChn;
        }
        data[1].ToLong(&xCoord);
        data[2].ToLong(&yCoord);
        if (data.size() > 3) {
            stringType = data[3];
        } else {
            stringType = rgbOrder;
        }
        Nodes.push_back(NodeBaseClassPtr(createNode(1, stringType, 1, stringType)));
        Nodes.back()->StringNum = 0;
        Nodes.back()->ActChan = actChn;
        Nodes.back()->Coords[0].bufX = xCoord;
        Nodes.back()->Coords[0].bufY = yCoord;
        lastActChn = actChn;
        for(size_t i=1; i < coordinateCount; i++) {
            data=wxSplit(wholeHouseDataArr[i],',');
            data[0].ToLong(&actChn);
            data[1].ToLong(&xCoord);
            data[2].ToLong(&yCoord);
            if (data.size() > 3) {
                stringType = data[3];
            } else {
                stringType = rgbOrder;
            }
            if(actChn != lastActChn) {
                Nodes.push_back(NodeBaseClassPtr(createNode(1, stringType, 1, stringType)));
                Nodes.back()->StringNum = 0;
                Nodes.back()->ActChan = actChn;
                Nodes.back()->Coords[0].bufX = xCoord;
                Nodes.back()->Coords[0].bufY = yCoord;
            } else {
                Nodes.back()->AddBufCoord(xCoord,yCoord);
            }
            lastActChn = actChn;
        }
    }
    if (zeroBased && minChan != 0) {
        for (int x = 0; x < Nodes.size(); x++) {
            Nodes[x]->ActChan -= minChan;
        }
    }
}
wxXmlNode* ModelClass::GetModelXml() {
    return this->ModelXml;
}

int ModelClass::GetNumberFromChannelString(wxString sc) {
    int output = 1;
    if (sc.Contains(":")) {
        output = wxAtoi(sc.SubString(0, sc.Find(":") - 1));
        sc = sc.SubString(sc.Find(":") + 1, sc.size());
    }
    int returnChannel = wxAtoi(sc);
    if (output > 1) {
        returnChannel = ModelNetInfo->CalcAbsChannel(output - 1, returnChannel - 1) + 1;
    }
    return returnChannel;
}

void ModelClass::SetFromXml(wxXmlNode* ModelNode, NetInfoClass &netInfo, bool zeroBased) {
    wxString tempstr,channelstr;
    wxString customModel,WholeHouseData;
    long degrees, StartChannel;
    size_t i;
    long i2;

    ModelXml=ModelNode;
    ModelNetInfo = &netInfo;
    StrobeRate=0;
    Nodes.clear();

    name=ModelNode->GetAttribute("name");
    DisplayAs=ModelNode->GetAttribute("DisplayAs");
    if (ModelNode->HasAttribute("StringType")) {
        // post 3.1.4
        StringType=ModelNode->GetAttribute("StringType");
    } else {
        // 3.1.4 and earlier
        StringType=ModelNode->GetAttribute("Order","RGB")+" Nodes";
    }
    SingleNode=HasSingleNode(StringType);
    SingleChannel=HasSingleChannel(StringType);
    rgbOrder = SingleNode ? "RGB" : StringType.Left(3);

    if(ModelNode->HasAttribute("versionNumber")) {
        tempstr=ModelNode->GetAttribute("versionNumber");
        tempstr.ToLong(&ModelVersion);
    } else {
        ModelVersion=0;
    }

    tempstr=ModelNode->GetAttribute("parm1");
    tempstr.ToLong(&parm1);
    tempstr=ModelNode->GetAttribute("parm2");
    tempstr.ToLong(&parm2);
    tempstr=ModelNode->GetAttribute("parm3");
    tempstr.ToLong(&parm3);
    tempstr=ModelNode->GetAttribute("starSizes");
    starSizes.resize(0);
    while (tempstr.size() > 0) {
        wxString t2 = tempstr;
        if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(","));
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        } else {
            tempstr = "";
        }
        i2 = 0;
        t2.ToLong(&i2);
        if ( i2 > 0) {
            starSizes.resize(starSizes.size() + 1);
            starSizes[starSizes.size() - 1] = i2;
        }
    }
    tempstr=ModelNode->GetAttribute("StrandNames");
    strandNames.clear();
    while (tempstr.size() > 0) {
        wxString t2 = tempstr;
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr(1, tempstr.length());
        } else if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(",") - 1);
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        } else {
            tempstr = "";
        }
        strandNames.push_back(t2);
    }
    tempstr=ModelNode->GetAttribute("NodeNames");
    nodeNames.clear();
    while (tempstr.size() > 0) {
        wxString t2 = tempstr;
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr(1, tempstr.length());
        } else if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(",") - 1);
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        } else {
            tempstr = "";
        }
        nodeNames.push_back(t2);
    }

    StartChannel = GetNumberFromChannelString(ModelNode->GetAttribute("StartChannel","1"));
    if (ModelNode->HasAttribute("ModelBrightness")) {
        tempstr=ModelNode->GetAttribute("ModelBrightness");
        tempstr.ToLong(&ModelBrightness);
    }
    tempstr=ModelNode->GetAttribute("Dir");
    IsLtoR=tempstr != "R";
    if (ModelNode->HasAttribute("StartSide")) {
        tempstr=ModelNode->GetAttribute("StartSide");
        isBotToTop = (tempstr == "B");
    } else {
        isBotToTop=true;
    }
    customColor = xlColor(ModelNode->GetAttribute("CustomColor", "#000000"));

    long n;
    tempstr=ModelNode->GetAttribute("Antialias","0");
    tempstr.ToLong(&n);
    pixelStyle = n;
    tempstr=ModelNode->GetAttribute("PixelSize","2");
    tempstr.ToLong(&n);
    pixelSize = n;
    tempstr=ModelNode->GetAttribute("Transparency","0");
    tempstr.ToLong(&n);
    transparency = n;
    blackTransparency = wxAtoi(ModelNode->GetAttribute("BlackTransparency","0"));
    previewBrightness = wxAtoi(ModelNode->GetAttribute("PreviewBrightness","100"));

    MyDisplay=IsMyDisplay(ModelNode);

    tempstr=ModelNode->GetAttribute("offsetXpct","0");
    tempstr.ToDouble(&offsetXpct);
    if(offsetXpct<0 || offsetXpct>1) {
        offsetXpct = .5;
    }
    tempstr=ModelNode->GetAttribute("offsetYpct","0");
    tempstr.ToDouble(&offsetYpct);
    if(offsetYpct<0 || offsetYpct>1) {
        offsetYpct = .5;
    }
    tempstr=ModelNode->GetAttribute("PreviewScale");
    singleScale = false;
    if (tempstr == "") {
        PreviewScaleX = wxAtof(ModelNode->GetAttribute("PreviewScaleX", "0.3333"));
        PreviewScaleY = wxAtof(ModelNode->GetAttribute("PreviewScaleY", "0.3333"));
    } else {
        singleScale = true;
        tempstr.ToDouble(&PreviewScaleX);
        tempstr.ToDouble(&PreviewScaleY);
    }
    if(PreviewScaleX<0 || PreviewScaleX>1) {
        PreviewScaleX = .33;
    }
    if(PreviewScaleY<0 || PreviewScaleY>1) {
        PreviewScaleY = .33;
    }
    tempstr=ModelNode->GetAttribute("PreviewRotation","0");
    tempstr.ToLong(&degrees);

    PreviewRotation=degrees;
    if (ModelVersion == 0) {
        //PreviewRotation *= 3; //Fix for formerversion of model rotation
        ModelVersion = 1;
    }

    // calculate starting channel numbers for each string
    size_t NumberOfStrings= HasOneString(DisplayAs) ? 1 : parm1;
    int ChannelsPerString=parm2*GetNodeChannelCount(StringType);
    if (SingleChannel)
        ChannelsPerString=1;
    else if (SingleNode)
        ChannelsPerString=GetNodeChannelCount(StringType);

    if ("Arches" == DisplayAs) {
        SingleChannel = false;
        ChannelsPerString=GetNodeChannelCount(StringType) * parm2;
    } else if (ModelNode->HasAttribute("CustomModel")) {
        customModel = ModelNode->GetAttribute("CustomModel");
        int maxval=GetCustomMaxChannel(customModel);
        // fix NumberOfStrings
        if (SingleNode) {
            NumberOfStrings=maxval;
        } else {
            ChannelsPerString=maxval*GetNodeChannelCount(StringType);
        }
    }

    tempstr=ModelNode->GetAttribute("Advanced","0");
    bool HasIndividualStartChans=tempstr == "1";
    stringStartChan.clear();
    stringStartChan.resize(NumberOfStrings);
    for (i=0; i<NumberOfStrings; i++) {
        tempstr=StartChanAttrName(i);
        if (!zeroBased && HasIndividualStartChans && ModelNode->HasAttribute(tempstr)) {
            stringStartChan[i] = GetNumberFromChannelString(ModelNode->GetAttribute(tempstr, "1"))-1;
        } else {
            stringStartChan[i] = (zeroBased? 0 : StartChannel-1) + i*ChannelsPerString;
        }
    }


    // initialize model based on the DisplayAs value
    wxStringTokenizer tkz(DisplayAs, " ");
    wxString token = tkz.GetNextToken();
    if(DisplayAs=="WholeHouse") {
        WholeHouseData = ModelNode->GetAttribute("WholeHouseData");
        InitWholeHouse(WholeHouseData, zeroBased);
        CopyBufCoord2ScreenCoord();
    } else if (token == "Tree") {
        int firstStrand = 0;
        if (zeroBased && ModelNode->GetAttribute("exportFirstStrand") != "") {
            firstStrand = wxAtoi(ModelNode->GetAttribute("exportFirstStrand")) - 1;
        }
        if (firstStrand < 0) {
            firstStrand = 0;
        }
        InitVMatrix(firstStrand);
        token = tkz.GetNextToken();
        token.ToLong(&degrees);
        if (token == "Flat") {
            degrees = 0;
        } else if (token == "Ribbon") {
            degrees = -1;
        }
        SetTreeCoord(degrees);
    } else if (token == "Sphere") {
        InitSphere();
        CopyBufCoord2ScreenCoord();
    } else if (DisplayAs == "Custom") {
        InitCustomMatrix(customModel);
        CopyBufCoord2ScreenCoord();
    } else if (DisplayAs == "Vert Matrix") {
        InitVMatrix();
        CopyBufCoord2ScreenCoord();
    } else if (DisplayAs == "Horiz Matrix") {
        InitHMatrix();
        CopyBufCoord2ScreenCoord();
    } else if (DisplayAs == "Single Line") {
        InitLine();
        CopyBufCoord2ScreenCoord();
        //SetLineCoord();
    } else if (DisplayAs == "Arches") {
        InitArches();
        SetArchCoord();
    } else if (DisplayAs == "Window Frame") {
        InitFrame();
        CopyBufCoord2ScreenCoord();
    } else if (DisplayAs == "Star") {
        InitStar();
        CopyBufCoord2ScreenCoord();
    } else if (DisplayAs == "Wreath") {
        InitWreath();
        CopyBufCoord2ScreenCoord();
    }

    //if (DisplayAs != "Single Line")
    //{
    SetModelCoord(PreviewRotation);
    //}
    size_t NodeCount=GetNodeCount();
    for(size_t i=0; i<NodeCount; i++) {
        Nodes[i]->sparkle = rand() % 10000;
    }

    wxXmlNode *f = ModelNode->GetChildren();
    faceInfo.clear();
    while (f != nullptr) {
        if ("faceInfo" == f->GetName()) {
            wxString name = f->GetAttribute("Name", "SingleNode");
            wxString type = f->GetAttribute("Type", "SingleNode");
            if (name == "") {
                name = type;
                f->DeleteAttribute("Name");
                f->AddAttribute("Name", type);
            }
            if (!(type == "SingleNode" || type == "NodeRange" || type == "Matrix")) {
                if (type == "Coro") {
                    type = "SingleNode";
                } else {
                    type = "Matrix";
                }
                f->DeleteAttribute("Type");
                f->AddAttribute("Type", type);
            }

            wxXmlAttribute *att = f->GetAttributes();
            while (att != nullptr) {
                if (att->GetName() != "Name") {
                    faceInfo[name][att->GetName()] = att->GetValue();
                }
                att = att->GetNext();
            }
        }
        f = f->GetNext();
    }
}

void ModelClass::GetNodeChannelValues(size_t nodenum, unsigned char *buf) {
    Nodes[nodenum]->GetForChannels(buf);
}
void ModelClass::SetNodeChannelValues(size_t nodenum, const unsigned char *buf) {
    Nodes[nodenum]->SetFromChannels(buf);
}
xlColor ModelClass::GetNodeColor(size_t nodenum) {
    xlColor color;
    Nodes[nodenum]->GetColor(color);
    return color;
}


// only valid for rgb nodes and dumb strings (not traditional strings)
wxChar ModelClass::GetChannelColorLetter(wxByte chidx) {
    return rgbOrder.GetChar(chidx);
}

int ModelClass::GetLastChannel() {
    int LastChan=0;
    size_t NodeCount=GetNodeCount();
    for(size_t idx=0; idx<NodeCount; idx++) {
        LastChan=std::max(LastChan,Nodes[idx]->ActChan + Nodes[idx]->GetChanCount() - 1);
    }
    return LastChan;
}

void ModelClass::SetOffset(double xPct, double yPct) {
    offsetXpct=xPct;
    offsetYpct=yPct;
}


void ModelClass::AddOffset(double xPct, double yPct) {
    offsetXpct+=xPct;
    offsetYpct+=yPct;
}


void ModelClass::SetScale(double x, double y) {
    PreviewScaleX = x;
    PreviewScaleY = y;
    singleScale = false;
}

void ModelClass::GetScales(double &x, double &y) {
    x = PreviewScaleX;
    y = PreviewScaleY;
}

int ModelClass::GetPreviewRotation() {
    return PreviewRotation;
}
// initialize buffer coordinates
// parm1=NumStrings
// parm2=PixelsPerString
// parm3=StrandsPerString
void ModelClass::InitVMatrix(int firstExportStrand) {
    int y,x,idx,stringnum,segmentnum,yincr;
    if (parm3 > parm2) {
        parm3 = parm2;
    }
    int NumStrands=parm1*parm3;
    int PixelsPerStrand=parm2/parm3;
    int PixelsPerString=PixelsPerStrand*parm3;
    SetBufferSize(PixelsPerStrand,NumStrands);
    SetNodeCount(parm1,PixelsPerString, rgbOrder);
    SetRenderSize(PixelsPerStrand,NumStrands);

    // create output mapping
    if (SingleNode) {
        x=0;
        for (size_t n=0; n<Nodes.size(); n++) {
            Nodes[n]->ActChan = stringStartChan[n];
            y=0;
            yincr=1;
            for (size_t c=0; c<PixelsPerString; c++) {
                Nodes[n]->Coords[c].bufX=IsLtoR ? x : NumStrands-x-1;
                Nodes[n]->Coords[c].bufY=y;
                y+=yincr;
                if (y < 0 || y >= PixelsPerStrand) {
                    yincr=-yincr;
                    y+=yincr;
                    x++;
                }
            }
        }
    } else {
        StartChannelVector_t strandStartChan;
        strandStartChan.clear();
        strandStartChan.resize(NumStrands);
        for (int x = 0; x < NumStrands; x++) {
            stringnum = x / parm3;
            segmentnum = x % parm3;
            strandStartChan[x] = stringStartChan[stringnum] + segmentnum * PixelsPerStrand * 3;
        }
        if (firstExportStrand > 0 && firstExportStrand < NumStrands) {
            int offset = strandStartChan[firstExportStrand];
            for (int x = 0; x < NumStrands; x++) {
                strandStartChan[x] = strandStartChan[x] - offset;
                if (strandStartChan[x] < 0) {
                    strandStartChan[x] += (PixelsPerStrand * NumStrands * 3);
                }
            }
        }

        for (x=0; x < NumStrands; x++) {
            stringnum = x / parm3;
            segmentnum = x % parm3;
            for(y=0; y < PixelsPerStrand; y++) {
                idx=stringnum * PixelsPerString + segmentnum * PixelsPerStrand + y;
                Nodes[idx]->ActChan = strandStartChan[x] + y*3;
                Nodes[idx]->Coords[0].bufX=IsLtoR ? x : NumStrands-x-1;
                Nodes[idx]->Coords[0].bufY= isBotToTop == (segmentnum % 2 == 0) ? y:PixelsPerStrand-y-1;
                Nodes[idx]->StringNum=stringnum;
            }
        }
    }
}
void ModelClass::InitArches() {
    int NumArches=parm1;
    int SegmentsPerArch=parm2;

    SetBufferSize(NumArches,SegmentsPerArch);
    if (SingleNode) {
        SetNodeCount(NumArches * SegmentsPerArch, parm3,rgbOrder);
    } else {
        SetNodeCount(NumArches, SegmentsPerArch, rgbOrder);
        if (parm3 > 1) {
            for (int x = 0; x < Nodes.size(); x++) {
                Nodes[x]->Coords.resize(parm3);
            }
        }
    }
    SetRenderSize(NumArches,SegmentsPerArch);

    for (int y=0; y < NumArches; y++) {
        for(int x=0; x<SegmentsPerArch; x++) {
            int idx = y * SegmentsPerArch + x;
            Nodes[idx]->ActChan = stringStartChan[y] + x*GetNodeChannelCount(StringType);
            Nodes[idx]->StringNum=y;
            for(size_t c=0; c < GetCoordCount(idx); c++) {
                Nodes[idx]->Coords[c].bufX=IsLtoR ? x : SegmentsPerArch-x-1;
                Nodes[idx]->Coords[c].bufY=isBotToTop ? y : NumArches-y-1;
            }
        }
    }
}

// Set screen coordinates for arches
void ModelClass::SetArchCoord() {
    double xoffset,x,y;
    int numlights=parm1*parm2*parm3;
    size_t NodeCount=GetNodeCount();
    SetRenderSize(parm2*parm3,numlights*2);
    double midpt=parm2*parm3;
    midpt -= 1.0;
    midpt /= 2.0;
    for(size_t n=0; n<NodeCount; n++) {
        xoffset=Nodes[n]->StringNum*parm2*parm3*2 - numlights;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            double angle=-M_PI/2.0 + M_PI * ((double)(Nodes[n]->Coords[c].bufX * parm3 + c))/midpt/2.0;
            x=xoffset + midpt*sin(angle)*2.0+parm2*parm3;
            y=(parm2*parm3)*cos(angle);
            Nodes[n]->Coords[c].screenX=x;
            Nodes[n]->Coords[c].screenY=y-(RenderHt/2);
        }
    }
}
// initialize buffer coordinates
// parm1=NumStrings
// parm2=PixelsPerString
// parm3=StrandsPerString
void ModelClass::InitHMatrix() {
    int y,x,idx,stringnum,segmentnum,xincr;
    if (parm3 > parm2) {
        parm3 = parm2;
    }
    int NumStrands=parm1*parm3;
    int PixelsPerStrand=parm2/parm3;
    int PixelsPerString=PixelsPerStrand*parm3;
    SetBufferSize(NumStrands,PixelsPerStrand);
    SetNodeCount(parm1,PixelsPerString,rgbOrder);
    SetRenderSize(NumStrands,PixelsPerStrand);

    // create output mapping
    if (SingleNode) {
        y=0;
        for (size_t n=0; n<Nodes.size(); n++) {
            Nodes[n]->ActChan = stringStartChan[n];
            x=0;
            xincr=1;
            for (size_t c=0; c<PixelsPerString; c++) {
                Nodes[n]->Coords[c].bufX=x;
                Nodes[n]->Coords[c].bufY=isBotToTop ? y :NumStrands-y-1;
                x+=xincr;
                if (x < 0 || x >= PixelsPerStrand) {
                    xincr=-xincr;
                    x+=xincr;
                    y++;
                }
            }
        }
    } else {
        for (y=0; y < NumStrands; y++) {
            stringnum=y / parm3;
            segmentnum=y % parm3;
            for(x=0; x<PixelsPerStrand; x++) {
                idx=stringnum * PixelsPerString + segmentnum * PixelsPerStrand + x;
                Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand*3 + x*3;
                Nodes[idx]->Coords[0].bufX=IsLtoR != (segmentnum % 2 == 0) ? PixelsPerStrand-x-1 : x;
                Nodes[idx]->Coords[0].bufY= isBotToTop ? y :NumStrands-y-1;
                Nodes[idx]->StringNum=stringnum;
            }
        }
    }
}

int ModelClass::GetCustomMaxChannel(const wxString& customModel) {
    wxString value;
    wxArrayString cols;
    long val,maxval=0;
    wxString valstr;

    wxArrayString rows=wxSplit(customModel,';');
    for(size_t row=0; row < rows.size(); row++) {
        cols=wxSplit(rows[row],',');
        for(size_t col=0; col < cols.size(); col++) {
            valstr=cols[col];
            if (!valstr.IsEmpty() && valstr != "0") {
                valstr.ToLong(&val);
                maxval=std::max(val,maxval);
            }
        }
    }
    return maxval;
}
void ModelClass::InitCustomMatrix(const wxString& customModel) {
    wxString value;
    wxArrayString cols;
    long idx;
    int width=1;
    std::vector<int> nodemap;

    wxArrayString rows=wxSplit(customModel,';');
    int height=rows.size();
    int cpn = ChannelsPerNode();
    for(size_t row=0; row < rows.size(); row++) {
        cols=wxSplit(rows[row],',');
        if (cols.size() > width) width=cols.size();
        for(size_t col=0; col < cols.size(); col++) {
            value=cols[col];
            if (!value.IsEmpty() && value != "0") {
                value.ToLong(&idx);

                // increase nodemap size if necessary
                if (idx > nodemap.size()) {
                    nodemap.resize(idx, -1);
                }
                idx--;  // adjust to 0-based

                // is node already defined in map?
                if (nodemap[idx] < 0) {
                    // unmapped - so add a node
                    nodemap[idx]=Nodes.size();
                    SetNodeCount(1,0,rgbOrder);  // this creates a node of the correct class
                    Nodes.back()->StringNum=idx;
                    Nodes.back()->ActChan=stringStartChan[0] + idx * cpn;
                    if (idx < nodeNames.size()) {
                        Nodes.back()->SetName(nodeNames[idx]);
                    }
                    Nodes.back()->AddBufCoord(col,height - row - 1);
                } else {
                    // mapped - so add a coord to existing node
                    Nodes[nodemap[idx]]->AddBufCoord(col,height - row - 1);
                }
            }
        }
    }
    for (int x = 0; x < Nodes.size(); x++) {
        for (int y = x+1; y < Nodes.size(); y++) {
            if (Nodes[y]->StringNum < Nodes[x]->StringNum) {
                Nodes[x].swap(Nodes[y]);
            }
        }
    }
    for (int x = 0; x < Nodes.size(); x++) {
        Nodes[x]->SetName(GetNodeName(Nodes[x]->StringNum));
    }

    SetBufferSize(height,width);
}

double ModelClass::toRadians(long degrees) {
    return 2.0*M_PI*double(degrees)/360.0;
}

long ModelClass::toDegrees(double radians) {
    return (radians/(2*M_PI))*360.0;
}


// initialize screen coordinates for tree
void ModelClass::SetTreeCoord(long degrees) {
    double bufferX, bufferY;
    if (BufferWi < 2) return;
    if (BufferHt < 1) return; // June 27,2013. added check to not divide by zero
    if (degrees > 0) {
        double angle;
        RenderHt=1000;
        RenderWi=((double)RenderHt)/1.8;

        double radians=toRadians(degrees);
        double radius=RenderWi/2.0;
        double topradius=RenderWi/12;

        double StartAngle=-radians/2.0;
        double AngleIncr=radians/double(BufferWi);
        if (degrees > 180) {
            //shift a tiny bit to make the strands in back not line up exactly with the strands in front
            StartAngle += AngleIncr / 5.0;
        }
        double topYoffset = std::abs(0.2 * topradius * cos(M_PI));
        double ytop = RenderHt - topYoffset;
        double ybot = std::abs(0.2 * radius * cos(M_PI));

        size_t NodeCount=GetNodeCount();
        for(size_t n=0; n<NodeCount; n++) {
            size_t CoordCount=GetCoordCount(n);
            for(size_t c=0; c < CoordCount; c++) {
                bufferX=Nodes[n]->Coords[c].bufX;
                bufferY=Nodes[n]->Coords[c].bufY;
                angle=StartAngle + double(bufferX) * AngleIncr;
                double xb=radius * sin(angle);
                double xt=topradius * sin(angle);
                double yb = ybot - 0.2 * radius * cos(angle);
                double yt = ytop - 0.2 * topradius * cos(angle);
                double posOnString = (bufferY/(double)(BufferHt-1.0));

                Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                Nodes[n]->Coords[c].screenY = yb + (yt - yb) * posOnString - ((double)RenderHt)/2.0;
            }
        }
    } else {
        double treeScale = degrees == -1 ? 5.0 : 4.0;
        double botWid = BufferWi * treeScale;
        RenderHt=BufferHt * 2.0;
        RenderWi=(botWid + 2);

        double offset = 0.5;
        size_t NodeCount=GetNodeCount();
        for(size_t n=0; n<NodeCount; n++) {
            size_t CoordCount=GetCoordCount(n);
            if (degrees == -1) {
                for(size_t c=0; c < CoordCount; c++) {
                    bufferX=Nodes[n]->Coords[c].bufX;
                    bufferY=Nodes[n]->Coords[c].bufY;

                    double xt = (bufferX + offset - BufferWi/2.0) * 0.9;
                    double xb = (bufferX + offset - BufferWi/2.0) * treeScale;
                    double h = std::sqrt(RenderHt * RenderHt + (xt - xb)*(xt - xb));

                    double posOnString = (bufferY/(double)(BufferHt-1.0));
                    double newh = RenderHt * posOnString;
                    Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords[c].screenY = RenderHt * newh / h - ((double)RenderHt)/2.0;

                    posOnString = ((bufferY - 0.33)/(double)(BufferHt-1.0));
                    newh = RenderHt * posOnString;
                    Nodes[n]->Coords.push_back(Nodes[n]->Coords[c]);
                    Nodes[n]->Coords.back().screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords.back().screenY = RenderHt * newh / h - ((double)RenderHt)/2.0;

                    posOnString = ((bufferY + 0.33)/(double)(BufferHt-1.0));
                    newh = RenderHt * posOnString;
                    Nodes[n]->Coords.push_back(Nodes[n]->Coords[c]);
                    Nodes[n]->Coords.back().screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords.back().screenY = RenderHt * newh / h - ((double)RenderHt)/2.0;
                }

            } else {
                for(size_t c=0; c < CoordCount; c++) {
                    bufferX=Nodes[n]->Coords[c].bufX;
                    bufferY=Nodes[n]->Coords[c].bufY;

                    double xt = (bufferX + offset - BufferWi/2.0) * 0.9;
                    double xb = (bufferX + offset - BufferWi/2.0) * treeScale;
                    double posOnString = (bufferY/(double)(BufferHt-1.0));
                    Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords[c].screenY = RenderHt * posOnString - ((double)RenderHt)/2.0;
                }
            }
        }
    }
}

void ModelClass::InitSphere() {
    SetNodeCount(parm1,parm2,rgbOrder);
    int numlights=parm1*parm2;
    SetBufferSize(numlights+1,numlights+1);
    int LastStringNum=-1;
    int offset=numlights/2;
    double r=offset;
    int chan = 0,x,y;
    double pct=isBotToTop ? 0.5 : 0.0;          // % of circle, 0=top
    double pctIncr=1.0 / (double)numlights;     // this is cw
    if (IsLtoR != isBotToTop) pctIncr*=-1.0;    // adjust to ccw
    int ChanIncr=SingleChannel ?  1 : 3;
    size_t NodeCount=GetNodeCount();

    /*
    x	=	r * cos(phi);
    y	=	r * sin(phi);
    z	=	r * cos(phi)
*/
    for(size_t n=0; n<NodeCount; n++) {
        if (Nodes[n]->StringNum != LastStringNum) {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            x=r*sin(pct*2.0*M_PI) + offset + 0.5;
            y=r*cos(pct*2.0*M_PI) + offset + 0.5;
            Nodes[n]->Coords[c].bufX=x;
            Nodes[n]->Coords[c].bufY=y;
            pct+=pctIncr;
            if (pct >= 1.0) pct-=1.0;
            if (pct < 0.0) pct+=1.0;
        }
    }
}


// initialize buffer coordinates
// parm1=Number of Strings/Arches
// parm2=Pixels Per String/Arch
void ModelClass::InitLine() {
    int numLights = parm1 * parm2;
    SetNodeCount(parm1,parm2,rgbOrder);
    SetBufferSize(1,numLights);
    int LastStringNum=-1;
    int chan = 0,idx;
    int ChanIncr=SingleChannel ?  1 : 3;
    size_t NodeCount=GetNodeCount();

    idx = 0;
    for(size_t n=0; n<NodeCount; n++) {
        if (Nodes[n]->StringNum != LastStringNum) {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].bufX=IsLtoR ? idx : numLights-idx-1;
            Nodes[n]->Coords[c].bufY=0;
            idx++;
        }
    }
}

// parm3 is number of points
// top left=top ccw, top right=top cw, bottom left=bottom cw, bottom right=bottom ccw
void ModelClass::InitStar() {
    if (parm3 < 2) parm3=2; // need at least 2 arms
    SetNodeCount(parm1,parm2,rgbOrder);

    int maxLights = 0;
    int numlights=parm1*parm2;
    int cnt = 0;
    if (starSizes.size() == 0) {
        starSizes.resize(1);
        starSizes[0] = numlights;
    }
    for (int x = 0; x < starSizes.size(); x++) {
        if ((cnt + starSizes[x]) > numlights) {
            starSizes[x] = numlights - cnt;
        }
        cnt += starSizes[x];
        if (starSizes[x] > maxLights) {
            maxLights = starSizes[x];
        }
    }
    SetBufferSize(maxLights+1,maxLights+1);


    int LastStringNum=-1;
    int chan = 0,cursegment,nextsegment,x,y;
    int start = 0;

    for (int cur = 0; cur < starSizes.size(); cur++) {
        numlights = starSizes[cur];
        if (numlights == 0) {
            continue;
        }

        int offset=numlights/2;

        int coffset = (maxLights - numlights) / 2;
        /*
        if (cur > 0) {
            for (int f = cur; f > 0; f--) {
                int i = starSizes[f];
                int i2 = starSizes[f - 1];
                coffset += (i2 - i) / 2;
            }
        }
         */

        int numsegments=parm3*2;
        double segstart_x,segstart_y,segend_x,segend_y,segstart_pct,segend_pct,r,segpct,dseg;
        double dpct=1.0/(double)numsegments;
        double OuterRadius=offset;
        double InnerRadius=OuterRadius/2.618034;    // divide by golden ratio squared
        double pct=isBotToTop ? 0.5 : 0.0;          // % of circle, 0=top
        double pctIncr=1.0 / (double)numlights;     // this is cw
        if (IsLtoR != isBotToTop) pctIncr*=-1.0;    // adjust to ccw
        int ChanIncr=SingleChannel ?  1 : 3;
        for(size_t cnt=0; cnt<numlights; cnt++) {
            int n = cur;
            if (!SingleNode) {
                n = start + cnt;
            } else {
                n = cur;
                if (n >= Nodes.size()) {
                    n = Nodes.size() - 1;
                }
            }
            if (Nodes[n]->StringNum != LastStringNum) {
                LastStringNum=Nodes[n]->StringNum;
                chan=stringStartChan[LastStringNum];
            }
            Nodes[n]->ActChan=chan;
            if (!SingleNode) {
                chan+=ChanIncr;
            }
            size_t CoordCount=GetCoordCount(n);
            int lastx = 0, lasty = 0;
            for(size_t c=0; c < CoordCount; c++) {
                if (c >= numlights) {
                    Nodes[n]->Coords[c].bufX=lastx;
                    Nodes[n]->Coords[c].bufY=lasty;
                } else {
                    cursegment=(int)((double)numsegments*pct) % numsegments;
                    nextsegment=(cursegment+1) % numsegments;
                    segstart_pct=(double)cursegment / numsegments;
                    segend_pct=(double)nextsegment / numsegments;
                    dseg=pct - segstart_pct;
                    segpct=dseg / dpct;
                    r=cursegment%2==0 ? OuterRadius : InnerRadius;
                    segstart_x=r*sin(segstart_pct*2.0*M_PI);
                    segstart_y=r*cos(segstart_pct*2.0*M_PI);
                    r=nextsegment%2==0 ? OuterRadius : InnerRadius;
                    segend_x=r*sin(segend_pct*2.0*M_PI);
                    segend_y=r*cos(segend_pct*2.0*M_PI);
                    // now interpolate between segstart and segend
                    x=(segend_x - segstart_x)*segpct + segstart_x + offset + 0.5 + coffset;
                    y=(segend_y - segstart_y)*segpct + segstart_y + offset + 0.5 + coffset;
                    Nodes[n]->Coords[c].bufX=x;
                    Nodes[n]->Coords[c].bufY=y;
                    lastx = x;
                    lasty = y;
                    pct+=pctIncr;
                    if (pct >= 1.0) pct-=1.0;
                    if (pct < 0.0) pct+=1.0;
                }
            }
        }
        start += numlights;
    }
}

// top left=top ccw, top right=top cw, bottom left=bottom cw, bottom right=bottom ccw
void ModelClass::InitWreath() {
    SetNodeCount(parm1,parm2,rgbOrder);
    int numlights=parm1*parm2;
    SetBufferSize(numlights+1,numlights+1);
    int LastStringNum=-1;
    int offset=numlights/2;
    double r=offset;
    int chan = 0,x,y;
    double pct=isBotToTop ? 0.5 : 0.0;          // % of circle, 0=top
    double pctIncr=1.0 / (double)numlights;     // this is cw
    if (IsLtoR != isBotToTop) pctIncr*=-1.0;    // adjust to ccw
    int ChanIncr=SingleChannel ?  1 : 3;
    size_t NodeCount=GetNodeCount();
    for(size_t n=0; n<NodeCount; n++) {
        if (Nodes[n]->StringNum != LastStringNum) {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            x=r*sin(pct*2.0*M_PI) + offset + 0.5;
            y=r*cos(pct*2.0*M_PI) + offset + 0.5;
            Nodes[n]->Coords[c].bufX=x;
            Nodes[n]->Coords[c].bufY=y;
            pct+=pctIncr;
            if (pct >= 1.0) pct-=1.0;
            if (pct < 0.0) pct+=1.0;
        }
    }
}
// initialize screen coordinates
// parm1=Number of Strings/Arches
// parm2=Pixels Per String/Arch
void ModelClass::SetLineCoord() {
    double x,y;
    float idx=0;
    size_t NodeCount=GetNodeCount();
    int numlights=parm1*parm2;
    double half=numlights/2;
    SetRenderSize(numlights*2,numlights);

    for(size_t n=0; n<NodeCount; n++) {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            x=idx;
            x=IsLtoR ? x - half : half - x;
            y=0;
            Nodes[n]->Coords[c].screenX=x;
            Nodes[n]->Coords[c].screenY=y + numlights;
            idx++;
        }
    }
}


// initialize buffer coordinates
// parm1=Nodes on Top
// parm2=Nodes left and right
// parm3=Nodes on Bottom
void ModelClass::InitFrame() {
    int x,y,newx,newy;
    SetNodeCount(1,parm1+2*parm2+parm3,rgbOrder);
    int FrameWidth=std::max(parm1,parm3)+2;
    SetBufferSize(parm2,FrameWidth);   // treat as outside of matrix
    //SetBufferSize(1,Nodes.size());   // treat as single string
    SetRenderSize(parm2,FrameWidth);
    int chan=stringStartChan[0];
    int ChanIncr=SingleChannel ?  1 : 3;

    int xincr[4]= {0,1,0,-1}; // indexed by side
    int yincr[4]= {1,0,-1,0};
    x=IsLtoR ? 0 : FrameWidth-1;
    y=isBotToTop ? 0 : parm2-1;
    int dir=1;            // 1=clockwise
    int side=x>0 ? 2 : 0; // 0=left, 1=top, 2=right, 3=bottom
    int SideIncr=1;       // 1=clockwise
    if ((parm1 > parm3 && x>0) || (parm3 > parm1 && x==0)) {
        // counter-clockwise
        dir=-1;
        SideIncr=3;
    }

    // determine starting position
    if (parm1 > parm3) {
        // more nodes on top, must start at bottom
        y=0;
    } else if (parm3 > parm1) {
        // more nodes on bottom, must start at top
        y=parm2-1;
    } else {
        // equal top and bottom, can start in any corner
        // assume clockwise numbering
        if (x>0 && y==0) {
            // starting in lower right
            side=3;
        } else if (x==0 && y>0) {
            // starting in upper left
            side=1;
        }
    }

    size_t NodeCount=GetNodeCount();
    for(size_t n=0; n<NodeCount; n++) {
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].bufX=x;
            Nodes[n]->Coords[c].bufY=y;
            newx=x+xincr[side]*dir;
            newy=y+yincr[side]*dir;
            if (newx < 0 || newx >= FrameWidth || newy < 0 || newy >= parm2) {
                // move to the next side
                side=(side+SideIncr) % 4;
                newx=x+xincr[side]*dir;
                newy=y+yincr[side]*dir;
            }
            x=newx;
            y=newy;
        }
    }
}

void ModelClass::SetBufferSize(int NewHt, int NewWi) {
    BufferHt=NewHt;
    BufferWi=NewWi;
}

void ModelClass::SetRenderSize(int NewHt, int NewWi) {
    RenderHt=NewHt;
    RenderWi=NewWi;
}

// not valid for Frame or Custom
int ModelClass::NodesPerString() {
    return SingleNode ? 1 : parm2;
}

int ModelClass::NodeStartChannel(size_t nodenum) const {
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->ActChan: 0; //avoid memory access error if no nods -DJ
}

wxString ModelClass::NodeType(size_t nodenum) const {
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->GetNodeType(): "RGB"; //avoid memory access error if no nods -DJ
}
int ModelClass::ChannelsPerNode() {
    return SingleChannel ? 1 : 3;
}

ModelClass::NodeBaseClass* ModelClass::createNode(int ns, const wxString &StringType, size_t NodesPerString, const wxString &rgbOrder) {
    if (StringType=="Single Color Red" || StringType == "R") {
        return new NodeClassRed(ns, NodesPerString);
    } else if (StringType=="Single Color Green" || StringType == "G") {
        return new NodeClassGreen(ns,NodesPerString);
    } else if (StringType=="Single Color Blue" || StringType == "B") {
        return new NodeClassBlue(ns,NodesPerString);
    } else if (StringType=="Single Color White" || StringType == "W") {
        return new NodeClassWhite(ns,NodesPerString);
    } else if (StringType.StartsWith("#")) {
        return new NodeClassCustom(ns,NodesPerString, xlColor(StringType));
    } else if (StringType=="Strobes White 3fps") {
        return new NodeClassWhite(ns,NodesPerString);
    } else if (StringType=="4 Channel RGBW" || StringType == "RGBW") {
        return new NodeClassRGBW(ns,NodesPerString);
    }
    return new NodeBaseClass(ns,1,rgbOrder);
}
wxString ModelClass::GetNextName() {
    if (nodeNames.size() > Nodes.size()) {
        return nodeNames[Nodes.size()];
    }
    return "";
}
// set size of Nodes vector and each Node's Coords vector
void ModelClass::SetNodeCount(size_t NumStrings, size_t NodesPerString, const wxString &rgbOrder) {
    size_t n;
    if (SingleNode) {
        if (StringType=="Single Color Red") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassRed(n,NodesPerString, GetNextName())));
        } else if (StringType=="Single Color Green") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassGreen(n,NodesPerString, GetNextName())));
        } else if (StringType=="Single Color Blue") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassBlue(n,NodesPerString, GetNextName())));
        } else if (StringType=="Single Color White") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassWhite(n,NodesPerString, GetNextName())));
        } else if (StringType=="Strobes White 3fps") {
            StrobeRate=7;  // 1 out of every 7 frames
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassWhite(n,NodesPerString, GetNextName())));
        } else if (StringType=="Single Color Custom") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassCustom(n,NodesPerString, customColor, GetNextName())));
        } else if (StringType=="4 Channel RGBW") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBW(n,NodesPerString, GetNextName())));
        } else {
            // 3 Channel RGB
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(n,NodesPerString, "RGB", GetNextName())));
        }
    } else if (NodesPerString == 0) {
        Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(0, 0, rgbOrder, GetNextName())));
    } else {
        size_t numnodes=NumStrings*NodesPerString;
        for(n=0; n<numnodes; n++)
            Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(n/NodesPerString, 1, rgbOrder, GetNextName())));
    }
}

int ModelClass::GetNodeChannelCount(const wxString & nodeType) {
    if (nodeType.StartsWith("Single Color")) {
        return 1;
    } else if (nodeType == "Strobes White 3fps") {
        return 1;
    } else if (nodeType == "4 Channel RGBW") {
        return 4;
    }
    return 3;
}


bool ModelClass::CanRotate() {
    return true; // DisplayAs == "Single Line";
}

void ModelClass::Rotate(int degrees) {
    if (!CanRotate()) return;
    PreviewRotation=degrees;
    SetLineCoord();
}

int ModelClass::GetRotation() {
    return PreviewRotation;
}


// returns a number where the first node is 1
int ModelClass::GetNodeNumber(size_t nodenum) {
    if (nodenum >= Nodes.size()) return 0;
    //if (Nodes[nodenum].bufX < 0) return 0;
    int sn=Nodes[nodenum]->StringNum;
    return (Nodes[nodenum]->ActChan - stringStartChan[sn]) / 3 + sn*NodesPerString() + 1;
}

size_t ModelClass::GetNodeCount() const {
    return Nodes.size();
}

int ModelClass::GetChanCount() const {
    size_t NodeCnt=GetNodeCount();
    if (NodeCnt == 0) {
        return 0;
    }
    int min = 999999999;
    int max = 0;

    for (int x = 0; x < NodeCnt; x++) {
        int i = Nodes[x]->ActChan;
        if (i < min) {
            min = i;
        }
        i += Nodes[x]->GetChanCount();
        if (i > max) {
            max = i;
        }
    }
    return max - min;
}
int ModelClass::GetChanCountPerNode() const {
    size_t NodeCnt=GetNodeCount();
    if (NodeCnt == 0) {
        return 0;
    }
    return Nodes[0]->GetChanCount();
}
size_t ModelClass::GetCoordCount(size_t nodenum) {
    return nodenum < Nodes.size() ? Nodes[nodenum]->Coords.size() : 0;
}

void ModelClass::GetNodeCoords(int nodeidx, std::vector<wxPoint> &pts) {
    for (int x = 0; x < Nodes[nodeidx]->Coords.size(); x++) {
        pts.push_back(wxPoint(Nodes[nodeidx]->Coords[x].bufX, Nodes[nodeidx]->Coords[x].bufY));
    }
}

bool ModelClass::IsCustom(void) {
    return (DisplayAs == "Custom");
}

//convert # to AA format so it matches Custom Model grid display:
//this makes it *so* much easier to visually compare with Custom Model grid display
//A - Z == 1 - 26
//AA - AZ == 27 - 52
//BA - BZ == 53 - 78
//etc
static wxString AA(int x) {
    wxString retval;
    --x;
//    if (x >= 26 * 26) { retval += 'A' + x / (26 * 26); x %= 26 * 26; }
    if (x >= 26) {
        retval += 'A' + x / 26 - 1;
        x %= 26;
    }
    retval += 'A' + x;
    return retval;
}
//add just the node#s to a choice list:
//NO add parsed info to choice list or check list box:
size_t ModelClass::GetChannelCoords(wxArrayString& choices) { //wxChoice* choices1, wxCheckListBox* choices2, wxListBox* choices3)
//    if (choices1) choices1->Clear();
//    if (choices2) choices2->Clear();
//    if (choices3) choices3->Clear();
//    if (choices1) choices1->Append(wxT("0: (none)"));
//    if (choices2) choices2->Append(wxT("0: (none)"));
//    if (choices3) choices3->Append(wxT("0: (none)"));
    size_t NodeCount = GetNodeCount();
    for (size_t n = 0; n < NodeCount; n++) {
        wxString newstr;
//        debug(10, "model::node[%d/%d]: #coords %d, ach# %d, str %d", n, NodeCount, Nodes[n]->Coords.size(), Nodes[n]->StringNum, Nodes[n]->ActChan);
        if (Nodes[n]->Coords.empty()) continue;
//        newstr = wxString::Format(wxT("%d"), GetNodeNumber(n));
//        choices.Add(newstr);
        choices.Add(GetNodeXY(n));
//        if (choices1) choices1->Append(newstr);
//        if (choices2) choices2->Append(newstr);
//        if (choices3)
//        {
//            wxArrayString strary;
//            choices3->InsertItems(strary, choices3->GetCount() + 0);
//        }
    }
    return choices.GetCount(); //choices1? choices1->GetCount(): 0) + (choices2? choices2->GetCount(): 0);
}
//get parsed node info:
wxString ModelClass::GetNodeXY(const wxString& nodenumstr) {
    long nodenum;
    size_t NodeCount = GetNodeCount();
    if (nodenumstr.ToLong(&nodenum))
        for (size_t inx = 0; inx < NodeCount; inx++) {
            if (Nodes[inx]->Coords.empty()) continue;
            if (GetNodeNumber(inx) == nodenum) return GetNodeXY(inx);
        }
    return nodenumstr; //not found?
}
wxString ModelClass::GetNodeXY(int nodeinx) {
    if ((nodeinx < 0) || (nodeinx >= GetNodeCount())) return wxEmptyString;
    if (Nodes[nodeinx]->Coords.empty()) return wxEmptyString;
    if (GetCoordCount(nodeinx) > 1) //show count and first + last coordinates
        if (IsCustom())
            return wxString::Format(wxT("%d: %d# @%s%d-%s%d"), GetNodeNumber(nodeinx), GetCoordCount(nodeinx), AA(Nodes[nodeinx]->Coords.front().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.front().bufY, AA(Nodes[nodeinx]->Coords.back().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.back().bufY); //NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
        else
            return wxString::Format(wxT("%d: %d# @(%d,%d)-(%d,%d"), GetNodeNumber(nodeinx), GetCoordCount(nodeinx), Nodes[nodeinx]->Coords.front().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.front().bufY, Nodes[nodeinx]->Coords.back().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.back().bufY); //NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
    else //just show singleton
        if (IsCustom())
            return wxString::Format(wxT("%d: @%s%d"), GetNodeNumber(nodeinx), AA(Nodes[nodeinx]->Coords.front().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.front().bufY);
        else
            return wxString::Format(wxT("%d: @(%d,%d)"), GetNodeNumber(nodeinx), Nodes[nodeinx]->Coords.front().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.front().bufY);
}

//extract first (X,Y) from string formatted above:
bool ModelClass::ParseFaceElement(const wxString& multi_str, std::vector<wxPoint>& first_xy) {
//    first_xy->x = first_xy->y = 0;
//    first_xy.clear();
    wxStringTokenizer wtkz(multi_str, "+");
    while (wtkz.HasMoreTokens()) {
        wxString str = wtkz.GetNextToken();
        if (str.empty()) continue;
        if (str.Find('@') == wxNOT_FOUND) continue; //return false;

        wxString xystr = str.AfterFirst('@');
        if (xystr.empty()) continue; //return false;
        long xval = 0, yval = 0;
        if (xystr[0] == '(') {
            xystr.Remove(0, 1);
            if (!xystr.BeforeFirst(',').ToLong(&xval)) continue; //return false;
            if (!xystr.AfterFirst(',').BeforeFirst(')').ToLong(&yval)) continue; //return false;
        } else {
            int parts = 0;
            while (!xystr.empty() && (xystr[0] >= 'A') && (xystr[0] <= 'Z')) {
                xval *= 26;
                xval += xystr[0] - 'A' + 1;
                xystr.Remove(0, 1);
                parts |= 1;
            }
            while (!xystr.empty() && (xystr[0] >= '0') && (xystr[0] <= '9')) {
                yval *= 10;
                yval += xystr[0] - '0';
                xystr.Remove(0, 1);
                parts |= 2;
            }
            if (parts != 3) continue; //return false;
            if (!xystr.empty() && (xystr[0] != '-')) continue; //return false;
        }
        wxPoint newxy(xval, yval);
        first_xy.push_back(newxy);
    }

    return !first_xy.empty(); //true;
}


wxString ModelClass::ChannelLayoutHtml() {
    size_t NodeCount=GetNodeCount();
    size_t i,idx;
    int n,x,y,s;
    wxString bgcolor;
    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi,0);
    bool IsCustom = DisplayAs == "Custom";
    wxString direction;
    if (IsCustom) {
        direction="n/a";
    } else if (!IsLtoR) {
        if(!isBotToTop)
            direction="Top Right";
        else
            direction="Bottom Right";
    } else {
        if (!isBotToTop)
            direction="Top Left";
        else
            direction="Bottom Left";
    }

    wxString html = "<html><body><table border=0>";
    html+="<tr><td>Name:</td><td>"+name+"</td></tr>";
    html+="<tr><td>Display As:</td><td>"+DisplayAs+"</td></tr>";
    html+="<tr><td>String Type:</td><td>"+StringType+"</td></tr>";
    html+="<tr><td>Start Corner:</td><td>"+direction+"</td></tr>";
    html+=wxString::Format("<tr><td>Total nodes:</td><td>%d</td></tr>",NodeCount);
    html+=wxString::Format("<tr><td>Height:</td><td>%d</td></tr>",BufferHt);
    html+="</table><p>Node numbers starting with 1 followed by string number:</p><table border=1>";


    int Ibufx,Ibufy;

    if (BufferHt == 1) {
        // single line or arch
        html+="<tr>";
        for(i=1; i<=NodeCount; i++) {
            n=IsLtoR ? i : NodeCount-i+1;
            s=Nodes[n-1]->StringNum+1;
            bgcolor=s%2 == 1 ? "#ADD8E6" : "#90EE90";
            html+=wxString::Format("<td bgcolor='"+bgcolor+"'>n%ds%d</td>",n,s);
        }
        html+="</tr>";
    } else if (BufferHt > 1) {
        // horizontal or vertical matrix or frame
        for(i=0; i<NodeCount; i++) {

            Ibufx = Nodes[i]->Coords[0].bufX;
            Ibufy = Nodes[i]->Coords[0].bufY;
            idx=Nodes[i]->Coords[0].bufY * BufferWi + Nodes[i]->Coords[0].bufX;
            if (idx < chmap.size()) chmap[idx]=i + 1;
        }
        for(y=BufferHt-1; y>=0; y--) {
            html+="<tr>";
            for(x=0; x<BufferWi; x++) {
                n=chmap[y*BufferWi+x];
                if (n==0) {
                    html+="<td></td>";
                } else {
                    s=Nodes[n-1]->StringNum+1;
                    bgcolor=s%2 == 1 ? "#ADD8E6" : "#90EE90";
                    html+=wxString::Format("<td bgcolor='"+bgcolor+"'>n%ds%d</td>",n,s);
                }
            }
            html+="</tr>";
        }
    } else {
        html+="<tr><td>Error - invalid height</td></tr>";
    }
    html+="</table></body></html>";
    return html;
}


// initialize screen coordinates
void ModelClass::CopyBufCoord2ScreenCoord() {
    size_t NodeCount=GetNodeCount();
    int xoffset=BufferWi/2;
    int yoffset=BufferHt/2;
    for(size_t n=0; n<NodeCount; n++) {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].screenX = Nodes[n]->Coords[c].bufX - xoffset;
            Nodes[n]->Coords[c].screenY = Nodes[n]->Coords[c].bufY - yoffset;
        }
    }
    SetRenderSize(BufferHt,BufferWi);
}

void ModelClass::UpdateXmlWithScale() {
    ModelXml->DeleteAttribute("offsetXpct");
    ModelXml->DeleteAttribute("offsetYpct");
    ModelXml->DeleteAttribute("PreviewScale");
    ModelXml->DeleteAttribute("PreviewScaleX");
    ModelXml->DeleteAttribute("PreviewScaleY");
    ModelXml->DeleteAttribute("PreviewRotation");
    ModelXml->DeleteAttribute("StartChannel");
    if (ModelXml->HasAttribute("versionNumber"))
        ModelXml->DeleteAttribute("versionNumber");
    ModelXml->AddAttribute("offsetXpct", wxString::Format("%6.4f",offsetXpct));
    ModelXml->AddAttribute("offsetYpct", wxString::Format("%6.4f",offsetYpct));
    if (singleScale) {
        ModelXml->AddAttribute("PreviewScale", wxString::Format("%6.4f",PreviewScaleX));
    } else {
        ModelXml->AddAttribute("PreviewScaleX", wxString::Format("%6.4f",PreviewScaleX));
        ModelXml->AddAttribute("PreviewScaleY", wxString::Format("%6.4f",PreviewScaleY));
    }
    ModelXml->AddAttribute("PreviewRotation", wxString::Format("%d",PreviewRotation));
    ModelXml->AddAttribute("versionNumber", wxString::Format("%ld",ModelVersion));
	ModelXml->AddAttribute("StartChannel", ModelStartChannel);
}

void ModelClass::AddToWholeHouseModel(ModelPreview* preview,std::vector<int>& xPos,std::vector<int>& yPos,
                                      std::vector<int>& actChannel, std::vector<wxString>& nodeTypes) {
    size_t NodeCount=Nodes.size();
    double sx,sy;
    int w, h;
    preview->GetVirtualCanvasSize(w,h);

    if (singleScale) {
        //we now have the virtual size so we can flip to non-single scale
        singleScale = false;
        if (RenderHt > RenderWi) {
            PreviewScaleX = double(RenderWi) * double(h) / (double(w) * RenderHt) * PreviewScaleY;
        } else {
            PreviewScaleY = double(RenderHt) * double(w) / (double(h) * RenderWi) * PreviewScaleX;
        }
    }
    double scalex = double(w) / RenderWi * PreviewScaleX;
    double scaley = double(h) / RenderHt * PreviewScaleY;
    double radians=toRadians(PreviewRotation);

    double w1 = int(offsetXpct*w);
    double h1 = int(offsetYpct*h);

    for(size_t n=0; n<NodeCount; n++) {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            sx=Nodes[n]->Coords[c].screenX;
            sy=Nodes[n]->Coords[c].screenY;

            sx = (sx*scalex);
            sy = (sy*scaley);
            TranslatePointDoubles(radians,sx,sy,sx,sy);
            sx += w1;
            sy += h1;

            xPos.push_back(sx);
            yPos.push_back(sy);
            actChannel.push_back(Nodes[n]->ActChan);
            nodeTypes.push_back(Nodes[n]->GetNodeType());
        }
    }
}

bool ModelClass::IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) {
    SetMinMaxModelScreenCoordinates(preview);
    int xs = x1<x2?x1:x2;
    int xf = x1>x2?x1:x2;
    int ys = y1<y2?y1:y2;
    int yf = y1>y2?y1:y2;

    if (mMinScreenX>=xs && mMaxScreenX<=xf && mMinScreenY>=ys && mMaxScreenY<=yf) {
        return true;
    } else {
        return false;
    }
}


bool ModelClass::HitTest(ModelPreview* preview,int x,int y) {
    int y1 = preview->GetVirtualCanvasHeight()-y;
    SetMinMaxModelScreenCoordinates(preview);
    if (x>=mMinScreenX && x<=mMaxScreenX && y1>=mMinScreenY && y1 <= mMaxScreenY) {
        return true;
    } else {
        return false;
    }
}

wxCursor ModelClass::GetResizeCursor(int cornerIndex) {
    int angleState;
    //LeftTop and RightBottom
    switch(cornerIndex) {
    // Left top when PreviewRotation = 0
    case 0:
        angleState = (int)(PreviewRotation/22.5);
        break;
    // Right Top
    case 1:
        angleState = ((int)(PreviewRotation/22.5)+4)%16;
        break;
    // Right Bottom
    case 2:
        angleState = ((int)(PreviewRotation/22.5)+8)%16;
        break;
    // Right Bottom
    default:
        angleState = ((int)(PreviewRotation/22.5)+12)%16;
        break;
    }
    switch(angleState) {
    case 0:
        return wxCURSOR_SIZENWSE;
    case 1:
        return wxCURSOR_SIZEWE;
    case 2:
        return wxCURSOR_SIZEWE;
    case 3:
        return wxCURSOR_SIZENESW;
    case 4:
        return wxCURSOR_SIZENESW;
    case 5:
        return wxCURSOR_SIZENS;
    case 6:
        return wxCURSOR_SIZENS;
    case 7:
        return wxCURSOR_SIZENWSE;
    case 8:
        return wxCURSOR_SIZENWSE;
    case 9:
        return wxCURSOR_SIZEWE;
    case 10:
        return wxCURSOR_SIZEWE;
    case 11:
        return wxCURSOR_SIZENESW;
    case 12:
        return wxCURSOR_SIZENESW;
    case 13:
        return wxCURSOR_SIZENS;
    case 14:
        return wxCURSOR_SIZENS;
    default:
        return wxCURSOR_SIZENWSE;
    }

}

int ModelClass::CheckIfOverHandles(ModelPreview* preview, wxCoord x,wxCoord y) {
    int status;
    if (x>mHandlePosition[0].x && x<mHandlePosition[0].x+RECT_HANDLE_WIDTH &&
            y>mHandlePosition[0].y && y<mHandlePosition[0].y+RECT_HANDLE_WIDTH) {
        preview->SetCursor(GetResizeCursor(0));
        status = OVER_L_TOP_HANDLE;
    } else if (x>mHandlePosition[1].x && x<mHandlePosition[1].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[1].y && y<mHandlePosition[1].y+RECT_HANDLE_WIDTH) {
        preview->SetCursor(GetResizeCursor(1));
        status = OVER_R_TOP_HANDLE;
    } else if (x>mHandlePosition[2].x && x<mHandlePosition[2].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[2].y && y<mHandlePosition[2].y+RECT_HANDLE_WIDTH) {
        preview->SetCursor(GetResizeCursor(2));
        status = OVER_R_BOTTOM_HANDLE;
    } else if (x>mHandlePosition[3].x && x<mHandlePosition[3].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[3].y && y<mHandlePosition[3].y+RECT_HANDLE_WIDTH) {
        preview->SetCursor(GetResizeCursor(3));
        status = OVER_R_BOTTOM_HANDLE;
    } else if (x>mHandlePosition[4].x && x<mHandlePosition[4].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[4].y && y<mHandlePosition[4].y+RECT_HANDLE_WIDTH) {
        preview->SetCursor(wxCURSOR_HAND);
        status = OVER_ROTATE_HANDLE;
    }

    else {
        preview->SetCursor(wxCURSOR_DEFAULT);
        status = OVER_NO_HANDLE;
    }
    return status;
}

// display model using colors stored in each node
// used when preview is running
void ModelClass::DisplayModelOnWindow(ModelPreview* preview, const xlColour *c, bool allowSelected) {
    size_t NodeCount=Nodes.size();
    double sx,sy;
    xlColour color;
    if (c != NULL) {
        color = *c;
    }
    int w, h;
    //int vw, vh;
    //preview->GetSize(&w, &h);
    preview->GetVirtualCanvasSize(w, h);

    if (pixelStyle == 1) {
        glEnable(GL_POINT_SMOOTH);
    }
    if (pixelSize != 2) {
        glPointSize(preview->calcPixelSize(pixelSize));
    }

    if (singleScale) {
        //we now have the virtual size so we can flip to non-single scale
        singleScale = false;
        if (RenderHt > RenderWi) {
            PreviewScaleX = double(RenderWi) * double(h) / (double(w) * RenderHt) * PreviewScaleY;
        } else {
            PreviewScaleY = double(RenderHt) * double(w) / (double(h) * RenderWi) * PreviewScaleX;
        }
    }

    double scalex=double(w) / RenderWi * PreviewScaleX;
    double scaley=double(h) / RenderHt * PreviewScaleY;

    double radians=toRadians(PreviewRotation);

    int w1 = int(offsetXpct*w);
    int h1 = int(offsetYpct*h);

    bool started = false;

    int first = 0; int last = NodeCount;
    int buffFirst = -1; int buffLast = -1;
    bool left = true;
    while (first < last) {
        int n = 0;
        if (left) {
            n = first;
            first++;
            if (buffFirst == -1) {
                buffFirst = Nodes[n]->Coords[0].bufX;
            }
            if (first < NodeCount && buffFirst != Nodes[first]->Coords[0].bufX) {
                left = false;
            }
        } else {
            last--;
            n = last;
            if (buffLast == -1) {
                buffLast = Nodes[n]->Coords[0].bufX;
            }
            if (last > 0 && buffFirst != Nodes[last - 1]->Coords[0].bufX) {
                left = true;
            }
        }
        if (c == NULL) {
            Nodes[n]->GetColor(color);
            if (previewBrightness != 100) {
                wxImage::HSVValue hsv = color.asHSV();
                hsv.value *= previewBrightness;
                hsv.value /= 100.0;
                if (hsv.value > 1.0) {
                    hsv.value = 1.0;
                }
                color = hsv;
            }
            if (StrobeRate) {
                int r = rand() % 5;
                if (r != 0) {
                    color = xlBLACK;
                }
            }
        }
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            // draw node on screen
            sx=Nodes[n]->Coords[c].screenX;;
            sy=Nodes[n]->Coords[c].screenY;
            sx = (sx*scalex);
            sy = (sy*scaley);
            TranslatePointDoubles(radians,sx,sy,sx,sy);
            sx += w1;
            sy += h1;

            if (pixelStyle < 2) {
                started = true;
                DrawGLUtils::AddVertex(sx,sy,color, color == xlBLACK ? blackTransparency : transparency);
            } else {
                int trans = transparency;
                if (color == xlBLACK) {
                    trans = blackTransparency;
                }
                DrawGLUtils::DrawCircle(color, sx, sy, pixelSize / 2,
                                        trans, pixelStyle == 2 ? transparency : 100);
            }
        }
    }
    if (started) {
        DrawGLUtils::End(GL_POINTS);
    }
    if (pixelStyle == 1) {
        glDisable(GL_POINT_SMOOTH);
    }
    if (pixelSize != 2) {
        glPointSize(preview->calcPixelSize(2));
    }


    if (Selected && c != NULL && allowSelected) {
        //Draw bounding rectangle
        // Upper Left Handle
        sx =  (-RenderWi*scalex/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
        sy = (RenderHt*scaley/2) + BOUNDING_RECT_OFFSET;
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx = sx + w1;
        sy = sy + h1;
        DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
        mHandlePosition[0].x = sx;
        mHandlePosition[0].y = sy;
        // Upper Right Handle
        sx =  (RenderWi*scalex/2) + BOUNDING_RECT_OFFSET;
        sy = (RenderHt*scaley/2) + BOUNDING_RECT_OFFSET;
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx = sx + w1;
        sy = sy + h1;
        DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
        mHandlePosition[1].x = sx;
        mHandlePosition[1].y = sy;
        // Lower Right Handle
        sx =  (RenderWi*scalex/2) + BOUNDING_RECT_OFFSET;
        sy = (-RenderHt*scaley/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx = sx + w1;
        sy = sy + h1;
        DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
        mHandlePosition[2].x = sx;
        mHandlePosition[2].y = sy;
        // Lower Left Handle
        sx =  (-RenderWi*scalex/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
        sy = (-RenderHt*scaley/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx = sx + w1;
        sy = sy + h1;
        DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
        mHandlePosition[3].x = sx;
        mHandlePosition[3].y = sy;

        // Draw rotation handle square
        sx = -RECT_HANDLE_WIDTH/2;
        sy = ((RenderHt*scaley/2) + 50);
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx += w1;
        sy += h1;
        DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
        // Save rotate handle
        mHandlePosition[4].x = sx;
        mHandlePosition[4].y = sy;
        // Draw rotation handle from center to 25 over rendered height
        sx = 0;
        sy = ((RenderHt*scaley/2) + 50);
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx += w1;
        sy += h1;
        DrawGLUtils::DrawLine(xlWHITE,255,w1,h1,sx,sy,1.0);
    }
}

void ModelClass::DisplayEffectOnWindow(ModelPreview* preview, double pointSize) {
    xlColor color;
    int w, h;


    preview->GetSize(&w, &h);

    double scaleX = double(w) * 0.95 / RenderWi;
    double scaleY = double(h) * 0.95 / RenderHt;
    double scale=scaleY < scaleX ? scaleY : scaleX;

    bool success = preview->StartDrawing(pointSize);

    if (pixelStyle == 1) {
        glEnable(GL_POINT_SMOOTH);
    }
    if (pixelSize != 2) {
        glPointSize(preview->calcPixelSize(pixelSize));
    }

    if(success) {
        // layer calculation and map to output
        size_t NodeCount=Nodes.size();
        double sx,sy;
        bool started = false;
        int first = 0; int last = NodeCount;
        int buffFirst = -1; int buffLast = -1;
        bool left = true;
        while (first < last) {
            int n = 0;
            if (left) {
                n = first;
                first++;
                if (buffFirst == -1) {
                    buffFirst = Nodes[n]->Coords[0].bufX;
                }
                if (first < NodeCount && buffFirst != Nodes[first]->Coords[0].bufX) {
                    left = false;
                }
            } else {
                last--;
                n = last;
                if (buffLast == -1) {
                    buffLast = Nodes[n]->Coords[0].bufX;
                }
                if (last > 0 && buffFirst != Nodes[last - 1]->Coords[0].bufX) {
                    left = true;
                }
            }

            Nodes[n]->GetColor(color);
            if (previewBrightness != 100) {
                wxImage::HSVValue hsv = color.asHSV();
                hsv.value *= previewBrightness;
                hsv.value /= 100.0;
                if (hsv.value > 1.0) {
                    hsv.value = 1.0;
                }
                color = hsv;
            }
            if (StrobeRate) {
                int r = rand() % 5;
                if (r != 0) {
                    color = xlBLACK;
                }
            }
            size_t CoordCount=GetCoordCount(n);
            for(size_t c=0; c < CoordCount; c++) {
                // draw node on screen
                sx=Nodes[n]->Coords[c].screenX;
                sy=Nodes[n]->Coords[c].screenY;

                double newsy = ((sy*scale)+(h/2));
                if (pixelStyle < 2) {
                    DrawGLUtils::AddVertex((sx*scale)+(w/2), newsy, color, color == xlBLACK ? blackTransparency : transparency);
                    started = true;
                } else {
                    int trans = transparency;
                    if (color == xlBLACK) {
                        trans = blackTransparency;
                    }
                    DrawGLUtils::DrawCircle(color, (sx*scale)+(w/2), newsy, pixelSize,
                                            trans, pixelStyle == 2 ? transparency : 100);
                }
            }
        }
        if (started) {
            DrawGLUtils::End(GL_POINTS);
        }
        preview->EndDrawing();
    }
    if (pixelStyle == 1) {
        glDisable(GL_POINT_SMOOTH);
    }
    if (pixelSize != 2) {
        glPointSize(preview->calcPixelSize(2));
    }
}


void ModelClass::SetModelCoord(int degrees) {
    PreviewRotation=degrees;
}

void ModelClass::SetMinMaxModelScreenCoordinates(ModelPreview* preview) {
    size_t NodeCount=Nodes.size();
    double sx,sy;
    int w, h;
    preview->GetVirtualCanvasSize(w, h);

    if (singleScale) {
        //we now have the virtual size so we can flip to non-single scale
        singleScale = false;
        if (RenderHt > RenderWi) {
            PreviewScaleX = double(RenderWi) * double(h) / (double(w) * RenderHt) * PreviewScaleY;
        } else {
            PreviewScaleY = double(RenderHt) * double(w) / (double(h) * RenderWi) * PreviewScaleX;
        }
    }

    double scalex = double(w) / RenderWi * PreviewScaleX;
    double scaley = double(h) / RenderHt * PreviewScaleY;
    double radians=toRadians(PreviewRotation);

    double w1 = int(offsetXpct*w);
    double h1 = int(offsetYpct*h);

    mMinScreenX = w;
    mMinScreenY = h;
    mMaxScreenX = 0;
    mMaxScreenY = 0;
    for(size_t n=0; n<NodeCount; n++) {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            // draw node on screen
            sx=Nodes[n]->Coords[c].screenX;
            sy=Nodes[n]->Coords[c].screenY;

            sx = (sx*scalex);
            sy = (sy*scaley);
            TranslatePointDoubles(radians,sx,sy,sx,sy);
            sx += w1;
            sy += h1;

            if (sx<mMinScreenX) {
                mMinScreenX = sx;
            }
            if (sx>mMaxScreenX) {
                mMaxScreenX = sx;
            }
            if (sy<mMinScreenY) {
                mMinScreenY = sy;
            }
            if (sy>mMaxScreenY) {
                mMaxScreenY = sy;
            }
        }
    }
    // Set minimum bounding rectangle
    if(mMaxScreenY-mMinScreenY<4) {
        mMaxScreenY+=2;
        mMinScreenY-=2;
    }
    if(mMaxScreenX-mMinScreenX<4) {
        mMaxScreenX+=2;
        mMinScreenX-=2;
    }
}

void ModelClass::ResizeWithHandles(ModelPreview* preview,int mouseX,int mouseY) {
    int w, h;
    // Get Center Point
    preview->GetVirtualCanvasSize(w, h);
    double w1 = offsetXpct* double(w);
    double h1 = offsetYpct* double(h);
    // Get mouse point in model space/ not screen space
    double sx,sy;
    sx = double(mouseX)-w1;
    sy = double(h-mouseY)-h1;
    double radians=-toRadians(PreviewRotation); // negative angle to reverse translation
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx = fabs(sx) - RECT_HANDLE_WIDTH;
    sy = fabs(sy) - RECT_HANDLE_WIDTH;
    SetScale( (double)(sx*2.0)/double(w), (double)(sy*2.0)/double(h));
}

void ModelClass::RotateWithHandles(ModelPreview* preview, bool ShiftKeyPressed, int mouseX,int mouseY) {
    int w, h;
    preview->GetVirtualCanvasSize(w, h);
    int w1 = int(offsetXpct*w);
    int h1 = int(offsetYpct*h);
    // Get mouse point in screen space where center of model is origin.
    int sx,sy;
    sx = mouseX-w1;
    sy = (h-mouseY)-h1;
    //Calculate angle of mouse from center.
    float tan = (float)sx/(float)sy;
    int angle = -toDegrees((double)atan(tan));
    if(sy>=0) {
        PreviewRotation = angle;
    } else if (sx<=0) {
        PreviewRotation = 90+(90+angle);
    } else {
        PreviewRotation = -90-(90-angle);
    }
    if(ShiftKeyPressed) {
        PreviewRotation = (int)(PreviewRotation/5) * 5;
    }
}

void ModelClass::SetTop(ModelPreview* preview,int y) {
    SetMinMaxModelScreenCoordinates(preview);
    int h = preview->GetVirtualCanvasHeight();
    int screenCenterY = h*offsetYpct;
    int newCenterY = screenCenterY + (y-mMaxScreenY);
    offsetYpct = ((float)newCenterY/(float)h);
}

void ModelClass::SetBottom(ModelPreview* preview,int y) {
    SetMinMaxModelScreenCoordinates(preview);
    int h = preview->GetVirtualCanvasHeight();
    int screenCenterY = h*offsetYpct;
    int newCenterY = screenCenterY + (y-mMinScreenY);
    offsetYpct = ((float)newCenterY/(float)h);
}

void ModelClass::SetLeft(ModelPreview* preview,int x) {
    SetMinMaxModelScreenCoordinates(preview);
    int w = preview->GetVirtualCanvasWidth();
    int screenCenterX = w*offsetXpct;
    int newCenterX = screenCenterX + (x-mMinScreenX);
    offsetXpct = ((float)newCenterX/(float)w);
}

void ModelClass::SetRight(ModelPreview* preview,int x) {
    SetMinMaxModelScreenCoordinates(preview);
    int w = preview->GetVirtualCanvasWidth();
    int screenCenterX = w*offsetXpct;
    int newCenterX = screenCenterX + (x-mMaxScreenX);
    offsetXpct = ((float)newCenterX/(float)w);
}

void ModelClass::SetHcenterOffset(float offset) {
    offsetXpct = offset;
}

void ModelClass::SetVcenterOffset(float offset) {
    offsetYpct = offset;
}

int ModelClass::GetTop(ModelPreview* preview) {
    SetMinMaxModelScreenCoordinates(preview);
    return mMaxScreenY;
}

int ModelClass::GetBottom(ModelPreview* preview) {
    SetMinMaxModelScreenCoordinates(preview);
    return mMinScreenY;
}

int ModelClass::GetLeft(ModelPreview* preview) {
    SetMinMaxModelScreenCoordinates(preview);
    return mMinScreenX;
}

int ModelClass::GetRight(ModelPreview* preview) {
    SetMinMaxModelScreenCoordinates(preview);
    return mMaxScreenX;
}

float ModelClass::GetHcenterOffset() {
    return offsetXpct;
}

float ModelClass::GetVcenterOffset() {
    return offsetYpct;
}

int ModelClass::GetStrandLength(int strand) const {
    if ("Custom" == DisplayAs) {
        return Nodes.size();
    } else if ("Star" == DisplayAs) {
        return SingleNode ? 1 : GetStarSize(strand);
    }
    return GetNodeCount() / GetNumStrands();
}

int ModelClass::MapToNodeIndex(int strand, int node) const {
    if ("Custom" == DisplayAs) {
        return node;
    } else if ("Star" == DisplayAs) {
        int idx = 0;
        for (int x = 0; x < strand; x++) {
            idx += GetStarSize(x);
        }
        idx += node;
        return idx;
    } else if ("Arches" == DisplayAs) {
        return strand * parm2 + node;
    } else if ((DisplayAs == wxT("Vert Matrix") || DisplayAs == wxT("Horiz Matrix")) && SingleChannel) {
        return node;
    }
    if (GetNumStrands() == 1) {
        return node;
    }
    if (SingleNode) {
        return strand;
    }
    return (strand * parm2 / parm3) + node;
}


void ModelClass::SetModelStartChan(wxString start_channel) {
	ModelStartChannel = start_channel;
}

