// Group2_UIDialogResource.cpp -- link bring-up body for
// UIDialog::ReadDialogResource (0x0046a920, ~16 KB), referenced by GameUi.cpp
// but defined nowhere yet. Best-guess admission (docs/GAME_BUILD.md), not
// byte-matched: extracted from samples/ui/GameUiNearMisses.cpp, whose control
// flow, calls and constants follow retail (92% of instructions equal with
// stack offsets and relocations ignored; the differences are register
// allocation and frame layout). The headers are GameUi.cpp's (plus the ones
// the sample adds), so the class layouts agree.
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GameUi.h"

#include "BackgroundImage.h"
#include "ContainerList.h"
#include "DebugAlloc.h"
#include "DialogEventKind.h"
#include "DialogProc.h"
#include "GUIManager.h"
#include "InputDevice.h"
#include "KeyboardDevice.h"
#include "MatrixUtil.h"
#include "PCAudio.h"
#include "PCTextureMap.h"
#include "Palette8.h"
#include "PCRenderTarget.h"
#include "RenderTarget.h"
#include "RenderInterfaces.h"
#include "TextureMap.h"
#include "Tgafile.h"
#include "TrackGame.h"
#include "UIDialog.h"
#include "UnknownResourceManager.h"

#include <imm.h>

#include "Display.h"
#include "Parameterblocks.h"

// The dialog resource parser

// 0x0065b600: the last multi-state text the parser passed on (its only
// retail reference is the store at 0x0046c43f; nothing reads it).
char* g_LastMultiStateText;

// 0x0046a920: reads the dialog resource at `offset` of `stream`. Its sections
// are listed between markers ("Set_Anim", "Set_Sound", "Set_Control") into
// the dialog's named entries (+0x9cc), then read in four passes: "Set_Info"
// (pass 0), the images (1), the sounds (2) and the controls with their
// "Set_Default" sections (3).
int UIDialog::ReadDialogResource(void* stream, int offset) {
    char backgroundImage[0x80];
    backgroundImage[0] = 0;
    int depth = g_TrackGame->renderTarget->surfacePixelFormat;
    memset(sectionTable, 0, sizeof(sectionTable));
    int setType = 1;
    UnknownParameterBlock parameters;
    parameters.Attach((UnknownParameterStream*)stream, offset, 1);
    char buffer[0x2800];
    parameters.GetSectionNames(buffer, sizeof(buffer));
    UnknownTokenizer* sections = new(__FILE__, 0x273) UnknownTokenizer(buffer);
    for (char* name = sections->NextToken("?"); name; name = sections->NextToken("?")) {
        if (!_strnicmp(name, "Set_Anim", strlen("Set_Anim"))) {
            setType = 1;
        } else if (!_strnicmp(name, "Set_Sound", strlen("Set_Sound"))) {
            setType = 2;
        } else if (!_strnicmp(name, "Set_Control", strlen("Set_Control"))) {
            setType = 4;
        } else {
            strcpy(sectionTable[sectionCount].sectionName, name);
            sectionTable[sectionCount].sectionObject = 0;
            if (!_strnicmp(name, "Set_Info", strlen("Set_Info")))
                sectionTable[sectionCount].field_0x38 = 0;
            else if (!_strnicmp(name, "Set_Default", strlen("Set_Default")))
                sectionTable[sectionCount].field_0x38 = 3;
            else
                sectionTable[sectionCount].field_0x38 = setType;
            sectionCount++;
        }
    }
    delete sections;

    // The "Set_Default" sections change these.
    int defaultMaxItems = 100;
    int defaultScale = 100;
    int defaultEnable = 1;
    int defaultShow = 1;
    int defaultSelectable = 1;
    int defaultTextColor = 0xffff;
    int defaultTextDrop = 1;
    int defaultColors[10];
    defaultColors[4] = 0xffff;
    int defaultSelectBoxColor = 0xff000000;
    int defaultItemBoxColor = 0xff000000;
    char defaultType[0x80];
    strcpy(defaultType, "");
    int defaultGroupId = 0;
    char defaultText[0x80];
    strcpy(defaultText, "");
    int defaultAttachId = 0;
    int defaultNumStates = 2;
    int defaultDefault = 0;
    int defaultAutoSort = 0;
    int defaultSelectColor = 0xff88ff;
    int defaultAllowWScroll = 0;
    int defaultTickNum = 0;
    int defaultFXDelay = 0;
    int defaultMaxLen = 1000;
    int defaultFontHeight = 0;
    int defaultDropColor = 0;
    int defaultKeyBind = 0;
    int defaultBold = 0;
    int defaultItalic = 0;
    int defaultSourceBlit = 0;
    int defaultPre3D = 0;
    int defaultPermanent = 0;
    int defaultShapeBounds = 0;
    int defaultListHeight = 20;
    int defaultRelAnchor = 0;
    int defaultMoveable = 0;
    defaultColors[0] = 0;
    defaultColors[1] = 0x8800;
    defaultColors[2] = 0x66ff;
    defaultColors[3] = 0xff6600;
    defaultColors[5] = 0xffffff;
    defaultColors[6] = 0xbbbbbb;
    defaultColors[7] = 0xbb55cc;
    defaultColors[8] = 0x880000;
    defaultColors[9] = 0x999999;
    char defaultFontName[0x80];
    strcpy(defaultFontName, "");
    char defaultMouseAnim[0x80];
    strcpy(defaultMouseAnim, "");
    char defaultSortLBName[0x80];
    strcpy(defaultSortLBName, "");
    char defaultAnchor[0x80];
    strcpy(defaultAnchor, "");
    char defaultTextAlign[0x80];
    strcpy(defaultTextAlign, "");
    char defaultTextAlignV[0x80];
    strcpy(defaultTextAlignV, "");
    char defaultFX[0x80];
    strcpy(defaultFX, "");
    char defaultFXSound[0x80];
    strcpy(defaultFXSound, "");
    char defaultFXAnimIn[0x80];
    strcpy(defaultFXAnimIn, "");
    char defaultFXAnimOut[0x80];
    strcpy(defaultFXAnimOut, "");
    char defaultFXSoundIn[0x80];
    strcpy(defaultFXSoundIn, "");
    char defaultFXSoundOut[0x80];
    strcpy(defaultFXSoundOut, "");
    char defaultAnimNorm[0x80];
    strcpy(defaultAnimNorm, "");
    char defaultAnimFocus[0x80];
    strcpy(defaultAnimFocus, "");
    char defaultAnimPush[0x80];
    strcpy(defaultAnimPush, "");
    char defaultAnimDisable[0x80];
    strcpy(defaultAnimDisable, "");
    char defaultAnimOff[0x80];
    strcpy(defaultAnimOff, "");
    char defaultAnimOn[0x80];
    strcpy(defaultAnimOn, "");
    char defaultSoundNorm[0x80];
    strcpy(defaultSoundNorm, "");
    char defaultSoundFocus[0x80];
    strcpy(defaultSoundFocus, "");
    char defaultSoundPush[0x80];
    strcpy(defaultSoundPush, "");
    char defaultSoundClick[0x80];
    strcpy(defaultSoundClick, "");
    char defaultSoundEnd[0x80];
    strcpy(defaultSoundEnd, "");
    char defaultAnimThumb[0x80];
    strcpy(defaultAnimThumb, "");
    char defaultAnimListBack[0x80];
    strcpy(defaultAnimListBack, "");
    char defaultAnimScrollBack[0x80];
    strcpy(defaultAnimScrollBack, "");
    char defaultAnimTextBack[0x80];
    strcpy(defaultAnimTextBack, "");
    char defaultStates[50][0x80];
    int n;
    for (n = 0; n < 50; n++)
        strcpy(defaultStates[n], "");
    char defaultStateTexts[50][0x80];
    for (n = 0; n < 50; n++)
        strcpy(defaultStateTexts[n], "");
    char defaultItems[50][0x80];
    for (n = 0; n < 50; n++)
        strcpy(defaultItems[n], "");

    // The current control's values (some keep the last control's).
    int value;
    int groupId;
    short item;
    CameraRect area;
    UIButton* button;
    UIMultiState* multiState;
    UIListBox* listBox;
    UIRadioButton* radioButton;
    UIStatic* staticControl;
    UIScrollBar* scrollBar;
    UIEditBox* editBox;
    UIStaticText* staticText;
    UIDropDownList* dropDownList;
    UIListBox* dropDownListBox;
    short state;
    int fontHeight;
    char type[0x80];
    UIControl* control;
    int isDefault;
    for (int pass = 0; pass < 4; pass++) {
        for (int i = 0; i < sectionCount; i++) {
            control = 0;
            isDefault = 0;
            parameters.SelectSection(sectionTable[i].sectionName);
            switch (sectionTable[i].field_0x38) {
            case 0:
                if (pass == 0) {
                    parameters.GetInt("ScreenWidth", g_TrackGame->renderTarget->targetWidth,
                                                     &screenWidth);
                    parameters.GetInt("ScreenHeight", g_TrackGame->renderTarget->targetHeight,
                                                     &screenHeight);
                    parameters.GetInt("NoScale", 0, &scaleToScreen);
                    scaleToScreen = !scaleToScreen;
                    parameters.GetInt("Popup", 1, &isPopup);
                    RenderTarget* target = (RenderTarget*)renderContext;
                    if (scaleToScreen) {
                        scaleX = (float)target->targetWidth / screenWidth;
                        scaleY = (float)target->targetHeight / screenHeight;
                    } else if (screenWidth * 2 > target->targetWidth ||
                               screenHeight * 2 > target->targetHeight) {
                        scaleX = scaleY = 1.0f;
                    }
                    if (isPopup) {
                        parameters.GetString("DlgAlignV", "MIDDLE", buffer, -1);
                        short align;
                        if (!_stricmp(buffer, "TOP"))
                            align = 8;
                        else
                            align = _stricmp(buffer, "BOTTOM") ? 0x10 : 0x20;
                        parameters.GetString("DlgAlignH", "CENTER", buffer, -1);
                        if (!_stricmp(buffer, "RIGHT"))
                            SetPopupAlignment(align | 4);
                        else if (!_stricmp(buffer, "LEFT"))
                            SetPopupAlignment(align | 1);
                        else
                            SetPopupAlignment(align | 2);
                    } else {
                        popupAlignment = 9;
                    }
                    for (int color = 0; color < 10; color++) {
                        sprintf(buffer, "%s%d", "TextColor", color);
                        parameters.GetInt(buffer, defaultColors[color], &value);
                        textColors[color] = value;
                    }
                    parameters.GetString("MouseCursorAnim", "", buffer, -1);
                    if (buffer[0])
                        cursorAnimation = (UnknownCursorAnimation*)FindSectionObject(buffer);
                    parameters.GetString("BackgroundFile", "", buffer, -1);
                    if (buffer[0] && dialogBackground) {
                        UnknownTgaFile* file = LoadFile(buffer, 0, (int)g_ResourceManager);
                        if (file) {
                            ownsBackground = 1;
                            if (parentDialog)
                                parentBackground = dialogBackground->currentImage;
                            dialogBackground->currentImage =
                                new(__FILE__, 0x367) PCTextureMap((TextureMapManager*)dialogTextures, 1);
                            dialogBackground->currentImage->CreateSurface(
                                file->bits, file->width, file->height, file->width, file->width, 0x22b, depth,
                                dialogPalette ? (UnknownTexturePalette*)((Palette8*)dialogPalette)->colorMapper : 0,
                                4, dialogPalette ? ((Palette8*)dialogPalette)->ddPalette : 0, 0, 0, 2, 1, 0,
                                0x80, 0xff00ff);
                            dialogBackground->MarkAllDirty();
                            FreeFile(file);
                        }
                    } else if (parentDialog) {
                        dialogBackground = parentDialog->dialogBackground;
                    }
                    char fontName[0x80];
                    parameters.GetString("FontName", "", fontName, -1);
                    float points;
                    parameters.GetFloat("PointSize", 0, &points);
                    int size = (int)points;
                    if (size < points)
                        size++;
                    parameters.GetFloat("FontHeight", 0, &points);
                    if (points > 0) {
                        size = (int)points;
                        if (size < points)
                            size++;
                    }
                    if (fontName[0] && size) {
                        if (dialogFont)
                            DeleteObject((HGDIOBJ)dialogFont);
                        dialogFontHeight = (int)(size * scaleY);
                        strcpy(dialogFontFace, fontName);
                        int bold;
                        int italic;
                        parameters.GetInt("Bold", 0, &bold);
                        parameters.GetInt("Italic", 0, &italic);
                        const char* face = guiManager ? guiManager->toolTipFontFace : "";
                        if (*face) {
                            strcpy(dialogFontFace, face);
                            bold = guiManager->toolTipBold;
                            italic = guiManager->toolTipItalic;
                        }
                        dialogFontHeight = (int)((guiManager ? *(float*)&guiManager->fontSizeOffset : 0.0f) +
                                                   dialogFontHeight);
                        dialogFont = CreateFontA(dialogFontHeight, 0, 0, 0, bold ? FW_BOLD : FW_MEDIUM, italic,
                                                         0, 0, DEFAULT_CHARSET, 0, 0, 2, 2, dialogFontFace);
                        dialogFontItalic = italic;
                        dialogFontBold = bold;
                    }
                    if (g_TrackGame->renderTarget->surfacePixelFormat == 8) {
                        int load = 1;
                        if (guiManager) {
                            if (guiManager->UnknownFunction4864f0())
                                load = 0;
                            dialogPalette = guiManager->guiPalette;
                        }
                        parameters.GetString("PaletteFile", "", buffer, -1);
                        if (buffer[0] && load) {
                            UnknownResourceEntry* resource = g_ResourceManager->FindItem(buffer, 1);
                            if (resource) {
                                dialogPalette = new(__FILE__, 0x3b6) Palette8(resource->archive);
                                ownsPalette = 1;
                            }
                        }
                        if (guiManager && load)
                            guiManager->SetGuiPalette((Palette8*)dialogPalette);
                    }
                    parameters.GetString("BackgroundImage", "", backgroundImage, -1);
                    if (backgroundImage[0]) {
                        CameraRect frameArea;
                        frameArea.left = 0;
                        frameArea.top = 0;
                        frameArea.right = screenArea.right - screenArea.left;
                        frameArea.bottom = screenArea.bottom - screenArea.top;
                        UIStatic* frame = new(__FILE__, 0x3cd) UIStatic(0, &frameArea, this);
                        frame->SetName("FRMBIStatic");
                        frame->permanent = 1;
                        controlContainer->AppendChild(frame, -1);
                        SetupControl(frame, 0, 0);
                    }
                }
                break;
            case 1:
                if (pass == 1 && imageCount < 500) {
                    imageTable[imageCount] = new(__FILE__, 0x3df) UIAnim(dialogTextures, (void*)1);
                    int delay;
                    int loops;
                    parameters.GetInt("MSecDelay", 0, &delay);
                    parameters.GetInt("LoopCount", 0, &loops);
                    imageTable[imageCount]->SetFrameDelay(delay);
                    imageTable[imageCount]->SetFrameCount(loops);
                    parameters.GetString("Files1", "", buffer, -1);
                    UnknownTokenizer files(buffer);
                    for (char* file = files.NextToken(","); file; file = files.NextToken(",")) {
                        if (g_ResourceManager->FindItem(sectionTable[i].sectionName, 1))
                            imageTable[imageCount]->LoadFromModule(
                                sectionTable[i].sectionName, (int)g_ResourceManager, (int)soundGroup,
                                dialogPalette);
                        else if (g_ResourceManager->FindItem(file, 1))
                            imageTable[imageCount]->LoadFromModule(
                                file, (int)g_ResourceManager, (int)soundGroup, dialogPalette);
                        else
                            imageTable[imageCount]->LoadFile(file, dialogPalette);
                    }
                    sectionTable[i].sectionObject = (Sound*)imageTable[imageCount];
                    imageCount++;
                }
                break;
            case 2:
                if (pass == 2 && soundCount < 20) {
                    int isStatic;
                    int copies;
                    parameters.GetInt("Static", 1, &isStatic);
                    parameters.GetInt("Copies", 0, &copies);
                    parameters.GetString("File", "", buffer, -1);
                    int flags = isStatic ? 1 : 2;
                    if (g_ResourceManager->FindItem(sectionTable[i].sectionName, 1)) {
                        soundTable[soundCount] =
                            LoadEntrySound(soundGroup, sectionTable[i].sectionName, flags, 3, copies, -1);
                    } else if (g_ResourceManager->FindItem(buffer, 1)) {
                        soundTable[soundCount] =
                            LoadEntrySound(soundGroup, buffer, flags, 3, copies, -1);
                    } else {
                        soundTable[soundCount] = new(__FILE__, 0x41e) Sound(soundGroup, 1);
                        soundTable[soundCount]->Load(buffer, 0, flags, 3, copies, -1);
                    }
                    sectionTable[i].sectionObject = soundTable[soundCount];
                    soundCount++;
                }
                break;
            case 3:
                if (pass != 3)
                    break;
                isDefault = 1;
            case 4: {
                if (pass != 3)
                    break;
                area.top = 0;
                area.left = 0;
                area.bottom = 0;
                area.right = 0;
                if (!isDefault) {
                    if (parameters.GetString("Xy", "", buffer, -1)) {
                        sscanf(buffer, "%d%*[,.] %d", &area.left, &area.top);
                        area.right = area.left;
                        area.bottom = area.top;
                    }
                    if (parameters.GetString("Xywh", "", buffer, -1)) {
                        sscanf(buffer, "%d%*[,.] %d%*[,.] %d%*[,.] %d", &area.left, &area.top, &area.right,
                               &area.bottom);
                        area.right += area.left;
                        area.bottom += area.top;
                    }
                    if (parameters.GetString("Xyxy", "", buffer, -1))
                        sscanf(buffer, "%d%*[,.] %d%*[,.] %d%*[,.] %d", &area.left, &area.top, &area.right,
                               &area.bottom);
                }
                parameters.GetInt("GroupId", defaultGroupId, isDefault ? &defaultGroupId : &groupId);
                parameters.GetString("Type", defaultType, isDefault ? defaultType : type, -1);

                if (!_stricmp(type, "BUTTON") || !_stricmp(type, "SCROLLUP") || !_stricmp(type, "SCROLLDOWN") ||
                    isDefault) {
                    if (!isDefault) {
                        if (!_stricmp(type, "BUTTON")) {
                            control = new(__FILE__, 0x45b) UIButton(0, &area, this);
                            button = (UIButton*)AddControl(control, groupId, 0);
                        } else if (!_stricmp(type, "SCROLLDOWN")) {
                            control = new(__FILE__, 0x45c) UIScrollCtl(10, 0, &area, this);
                            button = (UIButton*)AddControl(control, groupId, 0);
                        } else if (!_stricmp(type, "SCROLLUP")) {
                            control = new(__FILE__, 0x45d) UIScrollCtl(9, 0, &area, this);
                            button = (UIButton*)AddControl(control, groupId, 0);
                        }
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        button->SetNormalImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimFocus", defaultAnimFocus, isDefault ? defaultAnimFocus : buffer,
                                                     -1);
                    if (!isDefault)
                        button->SetFocusImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimPush", defaultAnimPush, isDefault ? defaultAnimPush : buffer, -1);
                    if (!isDefault)
                        button->SetPushImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimDisable", defaultAnimDisable,
                                                     isDefault ? defaultAnimDisable : buffer, -1);
                    if (!isDefault)
                        button->SetDisabledImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("SortLBName", defaultSortLBName,
                                                     isDefault ? defaultSortLBName : buffer, -1);
                    if (!isDefault)
                        button->SetSortList((UIListBox*)FindSectionObject(buffer));
                }

                if (!_stricmp(type, "MULTISTATE") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x474) UIMultiState(0, &area, this);
                        multiState = (UIMultiState*)AddControl(control, groupId, 0);
                    }
                    int states;
                    parameters.GetInt("NumStates", defaultNumStates,
                                                     isDefault ? &defaultNumStates : &states);
                    if (!isDefault)
                        static_cast<UIMultiState*>(multiState)->SetStateCount(states);
                    int selected;
                    parameters.GetInt("Default", defaultDefault, isDefault ? &defaultDefault : &selected);
                    if (!isDefault)
                        static_cast<UIMultiState*>(multiState)->SetCurrentState(selected);
                    state = 0;
                    char stateKey[0x80];
                    char textKey[0x80];
                    char stateBuffer[0x80];
                    char textBuffer[0x80];
                    sprintf(stateKey, "%s%d", "State", state);
                    sprintf(textKey, "%s%d", "StateText", state);
                    int gotState = parameters.GetString(stateKey, defaultStates[0],
                                                                    isDefault ? defaultStates[0] : stateBuffer, -1);
                    int gotText = parameters.GetString(textKey, defaultStateTexts[0],
                                                                   isDefault ? defaultStateTexts[0] : textBuffer, -1);
                    while (gotState | gotText) {
                        if (!isDefault) {
                            g_LastMultiStateText = textBuffer[0] ? textBuffer : 0;
                            multiState->SetStateEntry(state, (UIAnim*)FindSectionObject(stateBuffer),
                                                              g_LastMultiStateText);
                        }
                        state++;
                        // Retail tests the list items' counter here.
                        if (isDefault && item >= 50)
                            break;
                        sprintf(stateKey, "%s%d", "State", state);
                        sprintf(textKey, "%s%d", "StateText", state);
                        gotState = parameters.GetString(stateKey, state < 50 ? defaultStates[state] : "",
                                                                    isDefault ? defaultStates[state] : stateBuffer, -1);
                        gotText = parameters.GetString(textKey, state < 50 ? defaultStateTexts[state] : "",
                                                                   isDefault ? defaultStateTexts[state] : textBuffer, -1);
                    }
                }

                if (!_stricmp(type, "LISTBOX") || isDefault) {
                    parameters.GetInt("MaxItems", 100, isDefault ? &defaultMaxItems : &value);
                    if (!isDefault) {
                        control = new(__FILE__, 0x4a3) UIListBox(0, value, &area, this);
                        listBox = (UIListBox*)AddControl(control, groupId, 0);
                    }
                    parameters.GetInt("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SelectRow(value);
                    parameters.GetInt("AutoSort", defaultDefault, isDefault ? &defaultAutoSort : &value);
                    if (!isDefault)
                        listBox->SetAutoSort(value);
                    parameters.GetInt("SelectColor", defaultSelectColor,
                                                     isDefault ? &defaultSelectColor : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetSelectColor(value);
                    parameters.GetInt("Selectable", defaultSelectable,
                                                     isDefault ? &defaultSelectable : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetSelectable(value);
                    parameters.GetInt("AllowWScroll", defaultAllowWScroll,
                                                     isDefault ? &defaultAllowWScroll : &value);
                    if (!isDefault)
                        listBox->SetAllowWScroll(value);
                    parameters.GetInt("SelectBoxColor", defaultSelectBoxColor,
                                                     isDefault ? &defaultSelectBoxColor : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetSelectBoxColor(value);
                    parameters.GetInt("ItemBoxColor", defaultItemBoxColor,
                                                     isDefault ? &defaultItemBoxColor : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetItemBoxColor(value, -1);
                    char itemKey[0x80];
                    item = 1;
                    sprintf(itemKey, "%s%d", "Item", 1);
                    if (parameters.GetString(itemKey, defaultItems[1], isDefault ? defaultItems[1] : buffer,
                                                         -1)) {
                        do {
                            if (!isDefault) {
                                if (buffer[0] == '@')
                                    static_cast<UIListBox*>(listBox)->AddImageRow((UIAnim*)FindSectionObject(buffer + 1), 0, 0);
                                else
                                    static_cast<UIListBox*>(listBox)->AddRow(buffer, 0, 0);
                            }
                            item++;
                            if (isDefault && item >= 50)
                                break;
                            sprintf(itemKey, "%s%d", "Item", item);
                        } while (parameters.GetString(itemKey, item < 50 ? defaultItems[item] : "",
                                                                  isDefault ? defaultItems[item] : buffer, -1));
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault) {
                        listBox->SetImage(0, FindSectionObject(buffer));
                        listBox->SetImage(1, FindSectionObject(buffer));
                        listBox->SetImage(2, FindSectionObject(buffer));
                        listBox->SetImage(4, FindSectionObject(buffer));
                    }
                }

                if (!_stricmp(type, "RADIOBUTTON") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x4e3) UIRadioButton(0, &area, this);
                        radioButton = (UIRadioButton*)AddControl(control, groupId, 0);
                    }
                    parameters.GetInt("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault)
                        static_cast<UIMultiState*>(radioButton)->SetCurrentState(value);
                    parameters.GetString("AnimOff", defaultAnimOff, isDefault ? defaultAnimOff : buffer, -1);
                    if (!isDefault)
                        radioButton->SetStateEntry(0, (UIAnim*)FindSectionObject(buffer), 0);
                    parameters.GetString("AnimOn", defaultAnimOn, isDefault ? defaultAnimOn : buffer, -1);
                    if (!isDefault)
                        radioButton->SetStateEntry(1, (UIAnim*)FindSectionObject(buffer), 0);
                }

                if (!_stricmp(type, "STATIC") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x4f3) UIStatic(0, &area, this);
                        staticControl = (UIStatic*)AddControl(control, groupId, 0);
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        staticControl->SetImage(0, FindSectionObject(buffer));
                }

                if (!_stricmp(type, "VSCROLLBAR") || !_stricmp(type, "HSCROLLBAR") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x500) UIScrollBar(7, 0, &area, this);
                        scrollBar = (UIScrollBar*)AddControl(control, groupId, 0);
                    }
                    if (!_stricmp(type, "HSCROLLBAR"))
                        control->controlType = 8;
                    int ticks;
                    int position;
                    int scale;
                    parameters.GetInt("TickNum", defaultTickNum, isDefault ? &defaultTickNum : &ticks);
                    parameters.GetInt("Default", defaultDefault, isDefault ? &defaultDefault : &position);
                    parameters.GetInt("Scale", defaultScale, isDefault ? &defaultScale : &scale);
                    if (!isDefault) {
                        static_cast<UIScrollBar*>(scrollBar)->SetSnapStepCount(ticks);
                        static_cast<UIScrollBar*>(scrollBar)->SetScrollRange(scale);
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        scrollBar->trackImage = (UIAnim*)FindSectionObject(buffer);
                    parameters.GetString("AnimThumb", defaultAnimThumb, isDefault ? defaultAnimThumb : buffer,
                                                     -1);
                    if (!isDefault) {
                        scrollBar->SetImage(0, FindSectionObject(buffer));
                        scrollBar->SetImage(1, FindSectionObject(buffer));
                        scrollBar->SetImage(2, FindSectionObject(buffer));
                        scrollBar->SetImage(3, FindSectionObject(buffer));
                        if (control->bounds[2] == control->bounds[0]) {
                            UIAnim* image = control->stateImages[0];
                            control->bounds[2] = image->frameList[0]->frameWidth + control->bounds[0];
                            control->bounds[3] = control->bounds[1] + image->frameList[0]->frameHeight;
                        }
                        static_cast<UIScrollBar*>(scrollBar)->SetThumbPosition(position, ticks ? ticks
                                                                         : control->bounds[2] - control->bounds[0] + 1);
                        scrollBar->SetThumbSizeFromState(0);
                    }
                }

                if (!_stricmp(type, "EDITBOX") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x52c) UIEditBox(0, &area, this, 100, 0, 0);
                        editBox = (UIEditBox*)AddControl(control, groupId, 0);
                    }
                    parameters.GetInt("MaxLen", defaultMaxLen, isDefault ? &defaultMaxLen : &value);
                    if (!isDefault)
                        static_cast<UIEditBox*>(editBox)->SetCapacity(value);
                    parameters.GetString("SoundClick", defaultSoundClick,
                                                     isDefault ? defaultSoundClick : buffer, -1);
                    if (!isDefault)
                        editBox->SetClickSound((int)FindSectionObject(buffer));
                    parameters.GetString("SoundEnd", defaultSoundEnd, isDefault ? defaultSoundEnd : buffer, -1);
                    if (!isDefault)
                        editBox->SetFullSound((int)FindSectionObject(buffer));
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault) {
                        editBox->SetImage(0, FindSectionObject(buffer));
                        editBox->SetImage(1, FindSectionObject(buffer));
                        editBox->SetImage(2, FindSectionObject(buffer));
                        editBox->SetImage(4, FindSectionObject(buffer));
                    }
                }

                if (!_stricmp(type, "STATICTEXT") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x545) UIStaticText(0, &area, this, 0, 0xffff);
                        staticText = (UIStaticText*)AddControl(control, groupId, 0);
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        staticText->SetImage(0, FindSectionObject(buffer));
                }

                if (!_strnicmp(type, "DROPDOWNLIST", 12) || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x54f) UIDropDownList(0, &area, this, 0, 0xffff);
                        dropDownList = (UIDropDownList*)AddControl(control, groupId, 0);
                    }
                    parameters.GetString("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        dropDownList->buttonPart->SetNormalImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimFocus", defaultAnimFocus, isDefault ? defaultAnimFocus : buffer,
                                                     -1);
                    if (!isDefault)
                        dropDownList->buttonPart->SetFocusImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimPush", defaultAnimPush, isDefault ? defaultAnimPush : buffer, -1);
                    if (!isDefault)
                        dropDownList->buttonPart->SetPushImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimThumb", defaultAnimThumb, isDefault ? defaultAnimThumb : buffer,
                                                     -1);
                    if (!isDefault) {
                        UIScrollBar* thumb = dropDownList->scrollBarPart;
                        thumb->SetImage(0, FindSectionObject(buffer));
                        thumb->SetImage(1, FindSectionObject(buffer));
                        thumb->SetImage(2, FindSectionObject(buffer));
                        thumb->SetImage(3, FindSectionObject(buffer));
                        thumb->SetThumbSizeFromState(0);
                    }
                    parameters.GetInt("ListHeight", defaultListHeight,
                                                     isDefault ? &defaultListHeight : &value);
                    if (!isDefault) {
                        dropDownList->LayoutForRowHeight(value);
                        dropDownListBox = dropDownList->listPart;
                    }
                    char listKey[0x80];
                    item = 1;
                    sprintf(listKey, "%s%d", "Item", 1);
                    if (parameters.GetString(listKey, defaultItems[1], isDefault ? defaultItems[1] : buffer,
                                                         -1)) {
                        do {
                            if (!isDefault)
                                static_cast<UIListBox*>(dropDownListBox)->AddRow(buffer, 0, 0);
                            item++;
                            if (isDefault && item >= 50)
                                break;
                            sprintf(listKey, "%s%d", "Item", item);
                        } while (parameters.GetString(listKey, item < 50 ? defaultItems[item] : "",
                                                                  isDefault ? defaultItems[item] : buffer, -1));
                    }
                    control->SetName(sectionTable[i].sectionName);
                    int leftMargin;
                    int topMargin;
                    int textColor;
                    int dropColor;
                    parameters.GetInt("TextLMargin", 0, &leftMargin);
                    parameters.GetInt("TextTMargin", 0, &topMargin);
                    parameters.GetInt("TextColor", defaultTextColor, &textColor);
                    parameters.GetInt("DropColor", defaultDropColor, &dropColor);
                    char soundNorm[0x80];
                    char soundFocus[0x80];
                    char soundPush[0x80];
                    char soundClick[0x80];
                    parameters.GetString("SoundNorm", defaultSoundNorm, soundNorm, -1);
                    parameters.GetString("SoundFocus", defaultSoundFocus, soundFocus, -1);
                    parameters.GetString("SoundPush", defaultSoundPush, soundPush, -1);
                    parameters.GetString("SoundClick", defaultSoundClick, soundClick, -1);
                    if (!isDefault) {
                        dropDownList->buttonPart->SetSound(0, FindSectionObject(soundNorm));
                        dropDownList->buttonPart->SetSound(1, FindSectionObject(soundFocus));
                        dropDownList->buttonPart->SetSound(2, FindSectionObject(soundPush));
                        dropDownList->buttonPart->SetSound(3, FindSectionObject(soundClick));
                        dropDownList->textLeftMargin = leftMargin;
                        dropDownList->textTopMargin = topMargin;
                        dropDownList->SetFontColor(textColor);
                        dropDownList->SetDropColor(dropColor);
                        dropDownList->LayoutParts();
                    }
                    parameters.GetInt("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault) {
                        int row = value - 1 < 0 ? 0 : value - 1;
                        static_cast<UIListBox*>(dropDownListBox)->SelectRow(row);
                        dropDownList->SetText(static_cast<UIListBox*>(dropDownListBox)->GetRowText(row));
                    }
                    parameters.GetString("AnimTextBack", defaultAnimTextBack,
                                                     isDefault ? defaultAnimTextBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->SetCornerImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimListBack", defaultAnimListBack,
                                                     isDefault ? defaultAnimListBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->SetStaticImage((UIAnim*)FindSectionObject(buffer));
                    parameters.GetString("AnimScrollBack", defaultAnimScrollBack,
                                                     isDefault ? defaultAnimScrollBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->SetButtonImage((UIAnim*)FindSectionObject(buffer));
                }

                // Every control's settings.
                char fontName[0x80];
                parameters.GetString("FontName", defaultFontName, fontName, -1);
                float height;
                parameters.GetFloat("FontHeight", (float)defaultFontHeight, &height);
                int size = (int)height;
                if (size < height)
                    size++;
                if (isDefault)
                    defaultFontHeight = size;
                else
                    fontHeight = size;
                char anchor[0x80];
                int relAnchor;
                int moveable;
                char textAlign[0x80];
                char textAlignV[0x80];
                int textLMargin;
                int textTMargin;
                int fxDelay;
                char fx[0x80];
                char fxSound[0x80];
                char fxAnimIn[0x80];
                char fxSoundIn[0x80];
                char fxAnimOut[0x80];
                char fxSoundOut[0x80];
                char mouseAnim[0x80];
                char soundNorm[0x80];
                char soundFocus[0x80];
                char soundPush[0x80];
                char soundClick[0x80];
                char text[0x80];
                int attachId;
                int textColor;
                int dropColor;
                int keyBind;
                int italic;
                int bold;
                int textDrop;
                int sourceBlit;
                int enable;
                int show;
                int pre3D;
                int permanent;
                int shapeBounds;
                char toolTip[0x80];
                parameters.GetString("Anchor", defaultAnchor, isDefault ? defaultAnchor : anchor, -1);
                parameters.GetInt("RelAnchor", defaultRelAnchor, isDefault ? &defaultRelAnchor : &relAnchor);
                parameters.GetInt("Moveable", defaultMoveable, isDefault ? &defaultMoveable : &moveable);
                parameters.GetString("TextAlign", defaultTextAlign, isDefault ? defaultTextAlign : textAlign, -1);
                parameters.GetString("TextAlignV", defaultTextAlignV, isDefault ? defaultTextAlignV : textAlignV,
                                                 -1);
                parameters.GetInt("TextLMargin", 0, &textLMargin);
                parameters.GetInt("TextTMargin", 0, &textTMargin);
                parameters.GetInt("FXDelay", defaultFXDelay, isDefault ? &defaultFXDelay : &fxDelay);
                parameters.GetString("FX", defaultFX, isDefault ? defaultFX : fx, -1);
                parameters.GetString("FXSound", defaultFXSound, isDefault ? defaultFXSound : fxSound, -1);
                parameters.GetString("FXAnimIn", defaultFXAnimIn, isDefault ? defaultFXAnimIn : fxAnimIn, -1);
                parameters.GetString("FXSoundIn", defaultFXSoundIn, isDefault ? defaultFXSoundIn : fxSoundIn, -1);
                parameters.GetString("FXAnimOut", defaultFXAnimOut, isDefault ? defaultFXAnimOut : fxAnimOut, -1);
                parameters.GetString("FXSoundOut", defaultFXSoundOut, isDefault ? defaultFXSoundOut : fxSoundOut,
                                                 -1);
                parameters.GetString("MouseAnim", defaultMouseAnim, isDefault ? defaultMouseAnim : mouseAnim, -1);
                parameters.GetString("SoundNorm", defaultSoundNorm, isDefault ? defaultSoundNorm : soundNorm, -1);
                parameters.GetString("SoundFocus", defaultSoundFocus, isDefault ? defaultSoundFocus : soundFocus,
                                                 -1);
                parameters.GetString("SoundPush", defaultSoundPush, isDefault ? defaultSoundPush : soundPush, -1);
                parameters.GetString("SoundClick", defaultSoundClick, isDefault ? defaultSoundClick : soundClick,
                                                 -1);
                parameters.GetString("Text", defaultText, isDefault ? defaultText : text, -1);
                parameters.GetInt("AttachId", defaultAttachId, isDefault ? &defaultAttachId : &attachId);
                parameters.GetInt("TextColor", defaultTextColor, isDefault ? &defaultTextColor : &textColor);
                parameters.GetInt("DropColor", defaultDropColor, isDefault ? &defaultDropColor : &dropColor);
                parameters.GetInt("KeyBind", defaultKeyBind, isDefault ? &defaultKeyBind : &keyBind);
                parameters.GetInt("Italic", defaultItalic, isDefault ? &defaultItalic : &italic);
                parameters.GetInt("Bold", defaultBold, isDefault ? &defaultBold : &bold);
                parameters.GetInt("TextDrop", defaultTextDrop, isDefault ? &defaultTextDrop : &textDrop);
                parameters.GetInt("SourceBlit", defaultSourceBlit, isDefault ? &defaultSourceBlit : &sourceBlit);
                parameters.GetInt("Enable", defaultEnable, isDefault ? &defaultEnable : &enable);
                parameters.GetInt("Show", defaultShow, isDefault ? &defaultShow : &show);
                parameters.GetInt("Pre3D", defaultPre3D, isDefault ? &defaultPre3D : &pre3D);
                parameters.GetInt("Permanent", defaultPermanent, isDefault ? &defaultPermanent : &permanent);
                parameters.GetInt("ShapeBounds", defaultShapeBounds,
                                                 isDefault ? &defaultShapeBounds : &shapeBounds);
                parameters.GetString("ToolTipText", "", toolTip, -1);
                if (isDefault)
                    break;

                control->SetAttachId(attachId);
                control->groupId = groupId;
                control->SetAnchor((UIControl*)FindSectionObject(anchor), relAnchor);
                control->SetName(sectionTable[i].sectionName);
                control->moveable = moveable;
                if (control->bounds[0] == control->bounds[2]) {
                    UIAnim* image;
                    if (!_stricmp(type, "MULTISTATE") || !_stricmp(type, "RADIOBUTTON"))
                        image = ((UIMultiState*)control)->stateTable[((UIMultiState*)control)->selectedState].image;
                    else
                        image = control->stateImages[0];
                    control->bounds[2] = image->frameList[0]->frameWidth + control->bounds[0];
                    control->bounds[3] = control->bounds[1] + image->frameList[0]->frameHeight;
                }
                if (fontName[0] || (fontHeight && guiManager && guiManager->dialogFontName)) {
                    control->ownsFont = 1;
                    if (control->fontHandle)
                        DeleteObject((HGDIOBJ)control->fontHandle);
                    control->fontHandle = 0;
                    if (fontHeight)
                        control->fontHeight = fontHeight;
                    if (bold > 500)
                        bold = 1;
                    else if (bold > 1)
                        bold = 0;
                    const char* face = guiManager ? guiManager->toolTipFontFace : "";
                    if (*face) {
                        strcpy(control->fontFace, face);
                        bold = guiManager->toolTipBold;
                        italic = guiManager->toolTipItalic;
                    }
                    control->fontHeight = (int)((guiManager ? *(float*)&guiManager->fontSizeOffset : 0.0f) +
                                                 control->fontHeight);
                    if (control->fontHeight != dialogFontHeight || bold != dialogFontBold ||
                        italic != dialogFontItalic) {
                        control->fontHeight = (int)(control->fontHeight * scaleY);
                        control->fontHandle = (int)CreateFontA(control->fontHeight, 0, 0, 0, bold ? FW_BOLD : FW_MEDIUM,
                                                                italic, 0, 0, DEFAULT_CHARSET, 0, 0, 2, 2, dialogFontFace);
                        strcpy(control->fontFace, dialogFontFace);
                        control->bold = bold;
                        control->italic = italic;
                    }
                }
                if (sourceBlit)
                    control->sourceBlit = 1;
                control->SetShapeBounds(shapeBounds);
                if (guiManager->languageModule) {
                    int textId;
                    int toolTipId;
                    parameters.GetInt("LocalTextId", 0, &textId);
                    parameters.GetInt("LocalToolTipId", 0, &toolTipId);
                    if (textId)
                        control->SetTextFromResource(guiManager->languageModule, textId);
                    if (toolTipId) {
                        char tip[0x400];
                        if (LoadStringA((HINSTANCE)guiManager->languageModule, toolTipId, tip, sizeof(tip))) {
                            if (control->toolTipText)
                                DebugFree(control->toolTipText, __FILE__, 0x646);
                            control->toolTipText = (char*)DebugMalloc(strlen(tip) + 1, __FILE__, 0x647);
                            strcpy(control->toolTipText, tip);
                        } else {
                            sprintf(tip, "Resource string '%d' load fail\n", toolTipId);
                            if (control->toolTipText)
                                DebugFree(control->toolTipText, __FILE__, 0x64d);
                            control->toolTipText = 0;
                        }
                    }
                } else {
                    if (toolTip[0]) {
                        control->toolTipText = (char*)DebugMalloc(strlen(toolTip) + 1, __FILE__, 0x656);
                        strcpy(control->toolTipText, toolTip);
                    }
                    if (text[0])
                        control->SetText(text);
                }
                control->textLeftMargin = textLMargin;
                control->textTopMargin = textTMargin;
                control->SetTextDrop(textDrop != 0);
                control->SetFontColor(textColor);
                control->SetDropColor(dropColor);
                int align;
                if (!_stricmp(textAlignV, "TOP"))
                    align = 8;
                else
                    align = _stricmp(textAlignV, "BOTTOM") ? 0x10 : 0x20;
                if (!_stricmp(textAlign, "RIGHT")) {
                    align |= 4;
                } else if (!_stricmp(textAlign, "CENTER")) {
                    align |= 2;
                } else {
                    switch (control->controlType) {
                    case 1:
                    case 2:
                    case 4:
                    case 5:
                    case 9:
                    case 10:
                        align |= 2;
                        break;
                    default:
                        align |= 1;
                        break;
                    }
                }
                control->SetTextAlign(align);
                if (!_stricmp(fx, "GROW")) {
                    control->StartTransition(100, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "LSLIDE")) {
                    control->StartTransition(101, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "TSLIDE")) {
                    control->StartTransition(103, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "RSLIDE")) {
                    control->StartTransition(102, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "BSLIDE")) {
                    control->StartTransition(104, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "ANIM")) {
                    control->StartTransition(106, 0, 1, 0, -1);
                    control->fxAnimIn = (UIAnim*)FindSectionObject(fxAnimIn);
                    control->fxAnimOut = (UIAnim*)FindSectionObject(fxAnimOut);
                    control->fxSoundIn = FindSectionObject(fxSoundIn);
                    control->fxSoundOut = FindSectionObject(fxSoundOut);
                }
                control->SetSound(0, FindSectionObject(soundNorm));
                control->SetSound(1, FindSectionObject(soundFocus));
                control->SetSound(2, FindSectionObject(soundPush));
                control->SetSound(3, FindSectionObject(soundClick));
                control->SetEnabled(enable != 0);
                control->Show(show != 0, 1);
                control->mouseAnim = (int)FindSectionObject(mouseAnim);
                control->keyBind = keyBind;
                control->post3D = pre3D == 0;
                control->permanent = permanent;
                control->drawnTexture = control->GetStateTexture(-1);
                control->ScaleArea();
                sectionTable[i].sectionObject = (Sound*)control;
                break;
            }
            }
        }
    }

    if (backgroundImage[0]) {
        UIControl* frame = FindControl("FRMBIStatic", 5);
        UIAnim* image = (UIAnim*)FindSectionObject(backgroundImage);
        if (image) {
            int width = image->GetCurrentTexture()->textureWidth;
            int imageHeight = image->GetCurrentTexture()->textureHeight;
            frame->SetImage(0, image);
            frame->screenArea[2] = width;
            frame->screenArea[3] = imageHeight;
            *(CameraRect*)frame->bounds = *(CameraRect*)frame->screenArea;
            frame->drawnTexture = frame->GetStateTexture(0);
            screenArea.right = screenArea.left + width;
            screenArea.bottom = screenArea.top + imageHeight;
            frame->ScaleArea();
        }
    }
    if (isPopup && screenWidth && screenHeight) {
        int align = popupAlignment;
        RenderTarget* target = (RenderTarget*)renderContext;
        if (align & 1) {
            screenArea.right -= screenArea.left;
            screenArea.left = 0;
        } else if (align & 4) {
            int shift = target->targetWidth - screenArea.left * 2 - screenArea.right;
            screenArea.left += shift;
            screenArea.right += shift;
        } else {
            int shift = (target->targetWidth - screenArea.left - screenArea.right) / 2 -
                        screenArea.left;
            screenArea.left += shift;
            screenArea.right += shift;
        }
        if (align & 8) {
            screenArea.bottom -= screenArea.top;
            screenArea.top = 0;
        } else if (align & 0x20) {
            int shift = target->targetHeight - screenArea.top * 2 - screenArea.bottom;
            screenArea.top += shift;
            screenArea.bottom += shift;
        } else {
            int shift = (target->targetHeight - screenArea.top - screenArea.bottom) / 2 -
                        screenArea.top;
            screenArea.top += shift;
            screenArea.bottom += shift;
        }
    }
    return 1;
}

// Needed only by ReadDialogResource above and defined nowhere else yet:
// extracted from samples/ui/GameUiNearMisses.cpp (702 of 962 bytes match;
// the differences are store order/register choice, not behaviour).
// 0x0047a400: names the parts after the list ("<name>_BUT", "_LB", "_SB",
// "_LBK", "_SBK", "_TBK") and lays them out below and beside the text.
void UIDropDownList::LayoutParts() {
    int left = screenArea[0];
    CameraRect full;
    full.left = 0;
    full.top = 0;
    full.right = screenArea[2] - left;
    int top = screenArea[1];
    full.bottom = screenArea[3] - top;
    CameraRect button;
    UIAnim* image = buttonPart->stateImages[0];
    if (image) {
        button = full;
        button.left = full.right + 1;
        button.right = image->frameList[0]->frameWidth + button.left;
    } else {
        button.left = 0;
        button.top = 0;
        button.right = 0;
        button.bottom = 0;
    }
    CameraRect list = full;
    list.top = full.bottom + 1;
    list.bottom = rowHeight + list.top;
    CameraRect scroll = list;
    scroll.top += 2;
    scroll.bottom -= 2;
    scroll.left = button.left;
    scroll.right = button.right;
    CameraRect* area = (CameraRect*)openArea;
    char name[50];
    area->left = left;
    area->top = top;
    area->right = left + scroll.right;
    area->bottom = top + (list.bottom - 2);
    sprintf(name, "%s_BUT", GetName());
    buttonPart->SetName(name);
    *(CameraRect*)buttonPart->screenArea = button;
    *(CameraRect*)buttonPart->bounds = *(CameraRect*)buttonPart->screenArea;
    if (!button.left)
        buttonPart->Disable();
    sprintf(name, "%s_LB", GetName());
    listPart->SetName(name);
    *(CameraRect*)listPart->screenArea = list;
    *(CameraRect*)listPart->bounds = *(CameraRect*)listPart->screenArea;
    sprintf(name, "%s_SB", GetName());
    scrollBarPart->SetName(name);
    *(CameraRect*)scrollBarPart->screenArea = scroll;
    *(CameraRect*)scrollBarPart->bounds = *(CameraRect*)scrollBarPart->screenArea;
    sprintf(name, "%s_LBK", GetName());
    staticPart0->SetName(name);
    *(CameraRect*)staticPart0->screenArea = list;
    *(CameraRect*)staticPart0->bounds = *(CameraRect*)staticPart0->screenArea;
    sprintf(name, "%s_SBK", GetName());
    staticPart1->SetName(name);
    *(CameraRect*)staticPart1->screenArea = scroll;
    *(CameraRect*)staticPart1->bounds = *(CameraRect*)staticPart1->screenArea;
    sprintf(name, "%s_TBK", GetName());
    staticPart2->SetName(name);
    *(CameraRect*)staticPart2->screenArea = full;
    *(CameraRect*)staticPart2->bounds = *(CameraRect*)staticPart2->screenArea;
    listPart->textLeftMargin = textLeftMargin;
    listPart->textTopMargin = textTopMargin;
    static_cast<UIListBox*>(listPart)->SetRowTextColor(textColor, -1);
    if (textColor != 0xffffff)
        static_cast<UIListBox*>(listPart)->SetSelectColor(0xffffff);
    listPart->SetDropColor(dropColor);
}
