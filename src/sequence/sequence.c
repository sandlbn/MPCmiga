#include "../../include/sequence.h"
#include <string.h>
#include <stdio.h>

void InitSong(struct Song *song)
{
    memset(song, 0, sizeof(struct Song));
    song->bpm = 120;  // Default BPM
    song->currentSequence = 0;
    strcpy(song->name, "New Song");
    
    // Initialize all sequences
    for (int i = 0; i < MAX_SEQUENCES; i++) {
        sprintf(song->sequences[i].name, "Seq %d", i + 1);
    }
    song->sequences[0].active = TRUE;  // First sequence is active by default
}

void SetStep(struct Song *song, UBYTE seqNum, UBYTE trackNum, UBYTE stepNum,
             UBYTE note, UBYTE velocity, UBYTE volume)
{
    if (seqNum >= MAX_SEQUENCES || trackNum >= TRACKS_PER_SEQUENCE || 
        stepNum >= STEPS_PER_TRACK) return;
        
    struct Step *step = &song->sequences[seqNum].tracks[trackNum].steps[stepNum];
    step->note = note;
    step->velocity = velocity;
    step->volume = volume;
    step->active = TRUE;
}

void ClearStep(struct Song *song, UBYTE seqNum, UBYTE trackNum, UBYTE stepNum)
{
    if (seqNum >= MAX_SEQUENCES || trackNum >= TRACKS_PER_SEQUENCE || 
        stepNum >= STEPS_PER_TRACK) return;
        
    struct Step *step = &song->sequences[seqNum].tracks[trackNum].steps[stepNum];
    memset(step, 0, sizeof(struct Step));
}

void SetSequence(struct Song *song, UBYTE seqNum)
{
    if (seqNum >= MAX_SEQUENCES) return;
    song->currentSequence = seqNum;
}