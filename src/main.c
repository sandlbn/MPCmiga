#include "../include/gui.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>

int main(void)
{
    struct GUIState state;
    BOOL running = TRUE;

    if (!InitGUI(&state)) {
        printf("Failed to initialize GUI\n");
        return 20;
    }

    set(state.window, MUIA_Window_Open, TRUE);

    while (running) {
        ULONG signals = 0;
        ULONG id = DoMethod(state.app, MUIM_Application_NewInput, &signals);
        
        switch(id) {
            case MUIV_Application_ReturnID_Quit:
                running = FALSE;
                break;

            default:
                if (id > 0) {
                    HandleButtonPress(&state, id);
                }
                break;
        }
        
        if (running && signals) Wait(signals);
    }

    CleanupGUI(&state);
    return 0;
}
