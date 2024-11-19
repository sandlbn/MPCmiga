#ifndef GUI_H
#define GUI_H

#include <exec/types.h>
#include <libraries/mui.h>
#include <proto/muimaster.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <clib/alib_protos.h>
#include "sequence.h" 
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))

#define ID_PAD_BASE     100
#define ID_PREV_SEQUENCE 200
#define ID_NEXT_SEQUENCE 201
#define ID_STEP_BASE 300
#define ID_STEP_END (ID_STEP_BASE + (TRACKS_PER_SEQUENCE * STEPS_PER_TRACK))


// Mode definitions
enum {
    MODE_LOAD,
    MODE_SEQUENCE,
    MODE_EDIT,
    MODE_PITCH
};

struct GUIState {
    Object *app;
    Object *window;
    Object *bpmString;
    Object *bpmUpButton;
    Object *bpmDownButton;
    Object *loadButton;
    Object *sequenceButton;
    Object *editButton;
    Object *pitchButton;
    Object *playButton;
    Object *stopButton;
    Object *messageText1;
    Object *messageText2;
    Object *sequenceTracks[4 * 32];  // 4 tracks × 32 steps
    Object *samplePads[16];
    Object *trackGroups[4];  // Store track group objects
    Object *sequencePrevButton;
    Object *sequenceNextButton;
    Object *sequenceText;
    ULONG currentMode;
    LONG selectedTrack;    // -1 means none selected
    LONG selectedStep;     // -1 means none selected
    BOOL isPlaying;
    struct Song song;      // Now the compiler knows what struct Song is
};
struct StepMarker {
    int step;
    char *label;
    ULONG pen;  // Custom pen color
    BOOL isBold;
};

static const struct StepMarker stepMarkers[] = {
    {0,  "1",  1, TRUE},   // Step 1
    {3,  "4",  2, FALSE},  // Step 4
    {7,  "8",  1, TRUE},   // Step 8
    {11, "12", 2, FALSE},  // Step 12
    {15, "16", 1, TRUE},   // Step 16
    {19, "20", 2, FALSE},  // Step 20
    {23, "24", 1, TRUE},   // Step 24
    {27, "28", 2, FALSE},  // Step 28
    {31, "32", 1, TRUE},   // Step 32
    {-1, NULL, 0, FALSE}   // End marker
};

// Function prototypes
BOOL InitGUI(struct GUIState *state);
void CleanupGUI(struct GUIState *state);
void SetMessage1(struct GUIState *state, const char *text);
void SetMessage2(struct GUIState *state, const char *text);
void HandlePadPress(struct GUIState *state, ULONG padID);
void HandleButtonPress(struct GUIState *state, ULONG id);
void UpdateSequenceDisplay(struct GUIState *state);


// Helper macros
#define XGET(obj,attr) ({ \
    ULONG tmp; \
    get((obj),(attr),&tmp); \
    tmp; \
})

#endif