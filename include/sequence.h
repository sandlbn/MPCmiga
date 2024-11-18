#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <exec/types.h>

#define MAX_SEQUENCES 16
#define STEPS_PER_TRACK 32
#define TRACKS_PER_SEQUENCE 4

// Structure for a single note/step
struct Step {
    UBYTE note;        // MIDI note number (0-127)
    UBYTE velocity;    // Note velocity (0-127)
    UBYTE volume;      // Note volume (0-127)
    BOOL active;       // Whether this step is active
};

// Structure for a single track
struct Track {
    struct Step steps[STEPS_PER_TRACK];
    BOOL mute;
    BOOL solo;
    UBYTE sampleIndex;  // Which sample is assigned to this track
};

// Structure for a sequence
struct Sequence {
    struct Track tracks[TRACKS_PER_SEQUENCE];
    char name[16];     // Optional sequence name
    BOOL active;       // Whether this sequence is used
};

// Structure for the entire song
struct Song {
    struct Sequence sequences[MAX_SEQUENCES];
    UWORD bpm;
    UBYTE currentSequence;
    char name[32];
};

// Function prototypes
void InitSong(struct Song *song);
void SetStep(struct Song *song, UBYTE seqNum, UBYTE trackNum, UBYTE stepNum, 
             UBYTE note, UBYTE velocity, UBYTE volume);
void ClearStep(struct Song *song, UBYTE seqNum, UBYTE trackNum, UBYTE stepNum);
void SetSequence(struct Song *song, UBYTE seqNum);

#endif