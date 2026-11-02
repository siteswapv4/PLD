#pragma once
#include <PLD/PLD.h>
#include <PLD/PLD_image.h>
#include <PLD/PLD_note.h>
#include <PLD/PLD_utils.h>
#include <PLD/PLD_music.h>

typedef struct
{
	int thumbTimeStart;
	int thumbTimeEnd;
	int start;
	int end;
	int bpm;
    char* bpm_string;
    char* author;
    char* difficulties[PLD_DIFFICULTY_MAX];
}PLD_DataIni;

typedef struct
{
    bool autoplay;
    char* path;
    char* chartPath;
    int offset;
    PLD_Music* music;
    PLD_ArrayList* notes;
    PLD_ArrayList* currentNotes;
    PLD_DifficultyType difficulty;
    PLD_DataIni* dataIni;
}PLD_Song;

int PLD_AssignNotesImages(PLD_ArrayList* notes);

int PLD_PPDLinkChainslide(PLD_ArrayList* notes);

int PLD_LinkMulti(PLD_Note* multi[4], int count);

int PLD_LoadPPDChart(PLD_Song* songInfo);

PLD_DataIni* PLD_LoadDataIni(const char* path);

void PLD_DestroyDataIni(PLD_DataIni* data_ini);