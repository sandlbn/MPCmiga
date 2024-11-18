#include "../../include/gui.h"
#include "../../include/sequence.h"

#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <stdio.h>

struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;
struct Library *MUIMasterBase;


void UpdateSequenceDisplay(struct GUIState *state)
{
    char buffer[50];
    sprintf(buffer, "Sequence %d/16", state->song.currentSequence + 1);
    SetMessage2(state, buffer);
    
    // Update sequence text
    if (state->sequenceText) {
        set(state->sequenceText, MUIA_Text_Contents, buffer);
    }
}

// Add to HandleButtonPress:
void HandleButtonPress(struct GUIState *state, ULONG id)
{
    char buffer[100];
    if (id >= ID_PAD_BASE && id < ID_PAD_BASE + 16) {
        ULONG padNum = id - ID_PAD_BASE;
        sprintf(buffer, "Pad %ld pressed", padNum + 1);
        SetMessage1(state, buffer);
        return;
    }
    switch(id) {
        case ID_PREV_SEQUENCE:
            if (state->song.currentSequence > 0) {
                state->song.currentSequence--;
                UpdateSequenceDisplay(state);
            }
            break;

        case ID_NEXT_SEQUENCE:
            if (state->song.currentSequence < MAX_SEQUENCES - 1) {
                state->song.currentSequence++;
                UpdateSequenceDisplay(state);
            }
            break;

        case 2:  // Sequence
            state->currentMode = MODE_SEQUENCE;
            SetMessage1(state, "Sequence Mode: Press pads to trigger samples");
            break;

        case 3:  // Edit
            state->currentMode = MODE_EDIT;
            SetMessage1(state, "Edit Mode: Select step then pad to assign");
            break;

        case 4:  // Pitch
            state->currentMode = MODE_PITCH;
            SetMessage1(state, "Pitch Mode: Select step then set pitch");
            break;

        case 5:  // Play
            SetMessage1(state, "Playing sequence...");
            break;

        case 6:  // Stop
            SetMessage1(state, "Stopped");
            break;


        case MODE_SEQUENCE:
            state->currentMode = MODE_SEQUENCE;
            SetMessage1(state, "Sequence Mode: Press pads 1-16 to select sequence");
            break;

        // ... handle pad presses for sequence selection ...
        default:
            if (id >= ID_PAD_BASE && id < ID_PAD_BASE + 16) {
                ULONG padNum = id - ID_PAD_BASE;
                
                if (state->currentMode == MODE_SEQUENCE) {
                    // Use pad to select sequence
                    state->song.currentSequence = padNum;
                    UpdateSequenceDisplay(state);
                    sprintf(buffer, "Selected sequence %ld", padNum + 1);
                    SetMessage1(state, buffer);
                } else {
                    // Normal pad handling
                    sprintf(buffer, "Pad %ld pressed", padNum + 1);
                    SetMessage1(state, buffer);
                }
            }
            break;
    }
}


static BOOL InitLibs(void)
{
    if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39)))
        return FALSE;

    if (!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0))) {
        CloseLibrary((struct Library *)IntuitionBase);
        return FALSE;
    }

    if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, 19))) {
        CloseLibrary((struct Library *)GfxBase);
        CloseLibrary((struct Library *)IntuitionBase);
        return FALSE;
    }

    return TRUE;
}

static void CleanupLibs(void)
{
    if (MUIMasterBase) CloseLibrary(MUIMasterBase);
    if (GfxBase) CloseLibrary((struct Library *)GfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
}

static const struct StepMarker *GetStepMarker(int step) {
    for (int i = 0; stepMarkers[i].label != NULL; i++) {
        if (stepMarkers[i].step == step) {
            return &stepMarkers[i];
        }
    }
    return NULL;
}

static Object *CreateTrackGroup(struct GUIState *state, int trackNum)
{
    Object *group = HGroup,
        MUIA_Group_SameSize, TRUE,
        GroupSpacing(1),
    End;

    if (group) {
        // Add step buttons
        for (int i = 0; i < 32; i++) {
            const struct StepMarker *marker = GetStepMarker(i);
            char preparse[10] = "\33c";  // Default center alignment
            
            if (marker) {
                if (marker->isBold) {
                    strcat(preparse, "\33b");  // Add bold for important markers
                }
                
                state->sequenceTracks[trackNum * 32 + i] = TextObject, ButtonFrame,
                    MUIA_Background, MUII_ButtonBack,
                    MUIA_Text_Contents, marker->label,
                    MUIA_Text_PreParse, preparse,
                    MUIA_InputMode, MUIV_InputMode_RelVerify,
                    MUIA_Text_PreParse, "\33c",
                    MUIA_FixWidth, 12,
                    MUIA_FixHeight, 8,
                    MUIA_Weight, 0,
                    MUIA_Frame, MUIV_Frame_Button,
                End;
            } else {
                // Regular step button
                state->sequenceTracks[trackNum * 32 + i] = TextObject, ButtonFrame,
                    MUIA_Background, MUII_ButtonBack,
                    MUIA_Text_Contents, "-",
                    MUIA_Text_PreParse, "\33c",
                    MUIA_InputMode, MUIV_InputMode_RelVerify,
                    MUIA_FixWidth, 12,
                    MUIA_FixHeight, 8,
                    MUIA_Weight, 0,
                    MUIA_Frame, MUIV_Frame_Button,
                End;
            }

            if (state->sequenceTracks[trackNum * 32 + i]) {
                DoMethod(group, OM_ADDMEMBER, state->sequenceTracks[trackNum * 32 + i]);
            }
        }

        // Add M/S buttons
        Object *muteButton = TextObject, ButtonFrame,
            MUIA_Background, MUII_ButtonBack,
            MUIA_Text_Contents, "M",
            MUIA_Text_PreParse, "\33c",
            MUIA_FixWidth, 15,
            MUIA_FixHeight, 8,
            MUIA_InputMode, MUIV_InputMode_RelVerify,
        End;
        
        Object *soloButton = TextObject, ButtonFrame,
            MUIA_Background, MUII_ButtonBack,
            MUIA_Text_Contents, "S",
            MUIA_Text_PreParse, "\33c",
            MUIA_FixWidth, 15,
            MUIA_FixHeight, 8,
            MUIA_InputMode, MUIV_InputMode_RelVerify,
        End;

        if (muteButton && soloButton) {
            DoMethod(group, OM_ADDMEMBER, muteButton);
            DoMethod(group, OM_ADDMEMBER, soloButton);
        }
    }

    return group;

}


static Object *CreateMPCPads(struct GUIState *state)
{
    Object *padGroup = ColGroup(4),
        MUIA_Frame, MUIV_Frame_Group,
        MUIA_Background, MUII_GroupBack,
        MUIA_Group_SameSize, TRUE,
        MUIA_Group_Spacing, 8,    // Spacing between pads
        MUIA_Group_HorizSpacing, 8,
        MUIA_Group_VertSpacing, 8,
    End;

    if (padGroup) {
        for (int i = 0; i < 16; i++) {
            Object *pad = RectangleObject,  // Use Rectangle 
                ButtonFrame,
                MUIA_Background, MUII_ButtonBack,
                MUIA_Weight, 100,           // Equal weight for all dimensions
                MUIA_FixWidth, 50,          // Fixed minimum size
                MUIA_FixHeight, 50,         // Same as width for square
                MUIA_InputMode, MUIV_InputMode_RelVerify,
                MUIA_Draggable, FALSE,
                // Add centered S text
                Child, TextObject,
                    MUIA_Text_Contents, "S",
                    MUIA_Text_PreParse, "\33c",
                    MUIA_Weight, 0,
                End,
            End;

            if (pad) {
                state->samplePads[i] = pad;
                DoMethod(padGroup, OM_ADDMEMBER, pad);
                
                // Add notification for pad press
                DoMethod(pad, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
                        state->app, 3,
                        MUIM_Application_ReturnID, ID_PAD_BASE + i);
            }
        }
    }

    // Wrap the pad group in a larger group for better sizing
    Object *wrapper = VGroup,
        MUIA_Weight, 200,        // Give more weight to this section
        Child, padGroup,
        Child, HVSpace,          // Add space at bottom to push pads up
    End;

    return wrapper;
}
void HandlePadPress(struct GUIState *state, ULONG padID)
{
    ULONG padIndex = padID - ID_PAD_BASE;
    if (padIndex < 16) {
        // Handle pad press based on current mode
        switch(state->currentMode) {
            case MODE_LOAD:
                // Handle load mode
                SetMessage1(state, "Select sample to load...");
                // Add file requester here
                break;

            case MODE_SEQUENCE:
                // Handle sequence mode
                SetMessage1(state, "Pad triggered");
                // Add sample playback here
                break;

            case MODE_EDIT:
                // Handle edit mode
                if (state->selectedTrack >= 0 && state->selectedStep >= 0) {
                    char buffer[50];
                    sprintf(buffer, "Assigned pad %ld to step %d", padIndex + 1, state->selectedStep + 1);
                    SetMessage1(state, buffer);
                    // Update sequence data here
                }
                break;

            case MODE_PITCH:
                // Handle pitch mode
                if (state->selectedTrack >= 0 && state->selectedStep >= 0) {
                    char buffer[50];
                    sprintf(buffer, "Set pitch for pad %ld", padIndex + 1);
                    SetMessage1(state, buffer);
                    // Update pitch data here
                }
                break;
        }
    }
}


BOOL InitGUI(struct GUIState *state)
{

    memset(state, 0, sizeof(struct GUIState));

    // Initialize song data
    InitSong(&state->song);

    // Set initial values
    state->currentMode = MODE_SEQUENCE;
    state->selectedTrack = -1;
    state->selectedStep = -1;
    state->isPlaying = FALSE;

    if (!InitLibs()) return FALSE;

    state->app = ApplicationObject,
        MUIA_Application_Title, "Amiga Sampler",
        MUIA_Application_Version, "$VER: AmigaSampler 1.0 (17.11.2024)",
        MUIA_Application_Copyright, "©2024",
        MUIA_Application_Author, "Marcin Spoczynski",
        MUIA_Application_Description, "Sample Sequencer",
        MUIA_Application_Base, "AMIGASAMPLER",

        SubWindow, state->window = WindowObject,
            MUIA_Window_Title, "Amiga Sampler",
            MUIA_Window_ID, MAKE_ID('M','A','I','N'),

            WindowContents, VGroup,
                // BPM controls at top
                Child, HGroup,
                    Child, RectangleObject, End,  // Left spacer
                    Child, Label2("BPM:"),
                    Child, state->bpmString = StringObject,
                        MUIA_Frame, MUIV_Frame_String,
                        MUIA_String_MaxLen, 4,
                        MUIA_String_Contents, "120",
                        MUIA_FixWidthTxt, "250",
                    End,
                    Child, state->bpmUpButton = TextObject, ButtonFrame,
                        MUIA_Background, MUII_ButtonBack,
                        MUIA_Text_Contents, "+",
                        MUIA_FixWidthTxt, "+++",
                        MUIA_Text_PreParse, "\33c",
                        MUIA_InputMode, MUIV_InputMode_RelVerify,
                    End,
                    Child, state->bpmDownButton = TextObject, ButtonFrame,
                        MUIA_Background, MUII_ButtonBack,
                        MUIA_Text_Contents, "-",
                        MUIA_FixWidthTxt, "---",
                        MUIA_Text_PreParse, "\33c",
                        MUIA_InputMode, MUIV_InputMode_RelVerify,
                    End,
                End,

                // Control buttons
                Child, HGroup,
                    Child, state->loadButton = TextObject, ButtonFrame,
                        MUIA_Background, MUII_ButtonBack,
                        MUIA_Text_Contents, "Load",
                        MUIA_InputMode, MUIV_InputMode_RelVerify,
                    End,
                    Child, state->sequenceButton = TextObject, ButtonFrame,
                        MUIA_Background, MUII_ButtonBack,
                        MUIA_Text_Contents, "Sequence",
                        MUIA_InputMode, MUIV_InputMode_RelVerify,
                    End,
                    Child, state->editButton = TextObject, ButtonFrame,
                        MUIA_Background, MUII_ButtonBack,
                        MUIA_Text_Contents, "Edit",
                        MUIA_InputMode, MUIV_InputMode_RelVerify,
                    End,
                    Child, state->pitchButton = TextObject, ButtonFrame,
                        MUIA_Background, MUII_ButtonBack,
                        MUIA_Text_Contents, "Pitch",
                        MUIA_InputMode, MUIV_InputMode_RelVerify,
                    End,
                    Child, state->playButton = TextObject, ButtonFrame,
                        MUIA_Background, MUII_ButtonBack,
                        MUIA_Text_Contents, "Play",
                        MUIA_InputMode, MUIV_InputMode_RelVerify,
                    End,
                    Child, state->stopButton = TextObject, ButtonFrame,
                        MUIA_Background, MUII_ButtonBack,
                        MUIA_Text_Contents, "Stop",
                        MUIA_InputMode, MUIV_InputMode_RelVerify,
                    End,
                End,
        Child, state->messageText1 = TextObject,
            TextFrame,
            MUIA_Background, MUII_TextBack,
            MUIA_Text_Contents, "",
            MUIA_Text_PreParse, "\33c",
        End,
        
        Child, state->messageText2 = TextObject,
            TextFrame,
            MUIA_Background, MUII_TextBack,
            MUIA_Text_Contents, "",
            MUIA_Text_PreParse, "\33c",
        End,
        // Sequence tracks
        Child, VGroup,
            MUIA_Frame, MUIV_Frame_Group,
            MUIA_Background, MUII_GroupBack,
            
            Child, state->trackGroups[0] = CreateTrackGroup(state, 0),
            Child, state->trackGroups[1] = CreateTrackGroup(state, 1),
            Child, state->trackGroups[2] = CreateTrackGroup(state, 2),
            Child, state->trackGroups[3] = CreateTrackGroup(state, 3),
        End,
        // Sequence navigation
        Child, HGroup,
            Child, RectangleObject,
                MUIA_Weight, 100,    // Give most weight to this space to push content right
            End,
            Child, state->sequencePrevButton = TextObject, ButtonFrame,
                MUIA_Background, MUII_ButtonBack,
                MUIA_Text_Contents, "<",
                MUIA_Text_PreParse, "\33c",
                MUIA_InputMode, MUIV_InputMode_RelVerify,
                MUIA_FixWidth, 20,
                MUIA_Weight, 0,      // No expansion
            End,
            Child, state->sequenceText = TextObject,
                MUIA_Text_PreParse, "\33c",
                MUIA_Text_Contents, "Sequence 1/16",
                MUIA_Frame, MUIV_Frame_Text,
                MUIA_Weight, 0,      // No expansion
            End,
            Child, state->sequenceNextButton = TextObject, ButtonFrame,
                MUIA_Background, MUII_ButtonBack,
                MUIA_Text_Contents, ">",
                MUIA_Text_PreParse, "\33c",
                MUIA_InputMode, MUIV_InputMode_RelVerify,
                MUIA_FixWidth, 20,
                MUIA_Weight, 0,      // No expansion
            End,
            Child, RectangleObject,
                MUIA_Weight, 1,      // Small right margin
            End,
        End,

            Child, VGroup,
                GroupFrameT("Sample Pads"),
                Child, CreateMPCPads(state),
            End,
            End,
        End,
    End;
    for (int track = 0; track < 4; track++) {
        Object *trackGroup = (Object *)DoMethod(state->window, MUIM_FindObject, track + 1);
        
        // Add 32 step buttons
        for (int step = 0; step < 32; step++) {
            state->sequenceTracks[track * 32 + step] = TextObject, ButtonFrame,
                MUIA_Background, MUII_ButtonBack,
                MUIA_Text_Contents, "-",
                MUIA_Text_PreParse, "\33c",
                MUIA_InputMode, MUIV_InputMode_RelVerify,
                MUIA_FixWidth, 12,  
                MUIA_FixHeight, 8,  
                MUIA_Weight, 0,      
            End;
            DoMethod(trackGroup, OM_ADDMEMBER, state->sequenceTracks[track * 32 + step]);
        }
        
        // Add M/S buttons at a fixed size
        Object *muteButton = TextObject, ButtonFrame,
            MUIA_Background, MUII_ButtonBack,
            MUIA_Text_Contents, "M",
            MUIA_Text_PreParse, "\33c",
            MUIA_FixWidth, 15,
            MUIA_FixHeight, 8,
            MUIA_InputMode, MUIV_InputMode_RelVerify,
        End;
        
        Object *soloButton = TextObject, ButtonFrame,
            MUIA_Background, MUII_ButtonBack,
            MUIA_Text_Contents, "S",
            MUIA_Text_PreParse, "\33c",
            MUIA_FixWidth, 15,
            MUIA_FixHeight, 8,
            MUIA_InputMode, MUIV_InputMode_RelVerify,
        End;
        
        DoMethod(trackGroup, OM_ADDMEMBER, muteButton);
        DoMethod(trackGroup, OM_ADDMEMBER, soloButton);
    }

    if (!state->app) return FALSE;

    // Set up notifications
    DoMethod(state->loadButton, MUIM_Notify, MUIA_Pressed, FALSE,
            state->app, 2, MUIM_Application_ReturnID, 1);
            
    DoMethod(state->sequenceButton, MUIM_Notify, MUIA_Pressed, FALSE,
            state->app, 2, MUIM_Application_ReturnID, 2);
            
    DoMethod(state->editButton, MUIM_Notify, MUIA_Pressed, FALSE,
            state->app, 2, MUIM_Application_ReturnID, 3);
            
    DoMethod(state->pitchButton, MUIM_Notify, MUIA_Pressed, FALSE,
            state->app, 2, MUIM_Application_ReturnID, 4);
            
    DoMethod(state->playButton, MUIM_Notify, MUIA_Pressed, FALSE,
            state->app, 2, MUIM_Application_ReturnID, 5);
            
    DoMethod(state->stopButton, MUIM_Notify, MUIA_Pressed, FALSE,
            state->app, 2, MUIM_Application_ReturnID, 6);
    DoMethod(state->sequencePrevButton, MUIM_Notify, MUIA_Pressed, FALSE,
            state->app, 2, MUIM_Application_ReturnID, ID_PREV_SEQUENCE);
            
    DoMethod(state->sequenceNextButton, MUIM_Notify, MUIA_Pressed, FALSE,
            state->app, 2, MUIM_Application_ReturnID, ID_NEXT_SEQUENCE);

    // For each pad button:
    for (int i = 0; i < 16; i++) {
        DoMethod(state->samplePads[i], MUIM_Notify, MUIA_Pressed, FALSE,
                state->app, 2, MUIM_Application_ReturnID, ID_PAD_BASE + i);
    }

    DoMethod(state->window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
            state->app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

    return TRUE;
}
void CleanupGUI(struct GUIState *state)
{
    if (state->app) {
        MUI_DisposeObject(state->app);
        state->app = NULL;
    }
    CleanupLibs();
}

void SetMessage1(struct GUIState *state, const char *text)
{
    if (state->messageText1) {
        set(state->messageText1, MUIA_Text_Contents, text);
    }
}

void SetMessage2(struct GUIState *state, const char *text)
{
    if (state->messageText2) {
        set(state->messageText2, MUIA_Text_Contents, text);
    }
}

 