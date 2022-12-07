#ifndef GUIDATA_H
#define GUIDATA_H

#endif // GUIDATA_H


typedef struct {
    int vfoDisplayMode; //0: use Left/Right mouse button, 1: click digit Up or Down
    bool darkTheme; //flag for Dark theme
    bool peakHold;  //meters peak hold
} guiConfig;


typedef struct {
    int bwidthList;
    int antList;
    int rangeList;
    int tabList;
    int toneList;
} guiCommand;
