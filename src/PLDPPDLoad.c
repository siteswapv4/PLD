#include "PLDPPDLoad.h"

#define PLD_PPD_TYPE_ISID 40
#define PLD_PPD_TYPE_TRIANGLE 3
#define PLD_PPD_TYPE_SQUARE 0
#define PLD_PPD_TYPE_CROSS 1
#define PLD_PPD_TYPE_CIRCLE 2
#define PLD_PPD_TYPE_TRIANGLEHOLD 23
#define PLD_PPD_TYPE_SQUAREHOLD 20
#define PLD_PPD_TYPE_CROSSHOLD 21
#define PLD_PPD_TYPE_CIRCLEHOLD 22
#define PLD_PPD_TYPE_SLIDEL 9
#define PLD_PPD_TYPE_SLIDER 8
#define PLD_PPD_TYPE_CHAINL 29
#define PLD_PPD_TYPE_CHAINR 28

#define PLD_PPD_WIDTH 800
#define PLD_PPD_HEIGHT 450

typedef enum
{
    PLD_PPD_EVENT_CHANGEVOLUME,
    PLD_PPD_EVENT_CHANGEBPM,
    PLD_PPD_EVENT_CHANGEBPMRAPID,
    PLD_PPD_EVENT_CHANGESOUND,
    PLD_PPD_EVENT_CHANGEDISPLAY,
    PLD_PPD_EVENT_CHANGEMOVESTATE,
    PLD_PPD_EVENT_CHANGERELEASESOUND,
    PLD_PPD_EVENT_CHANGENOTETYPE,
    PLD_PPD_EVENT_CHANGEINITIALIZEORDER,
    PLD_PPD_EVENT_CHANGESLIDESCALE
} PLD_PPDEventType;

typedef struct
{
    int time;
    PLD_PPDEventType type;
    float BPM;
    float sliderScale;
    int value;
}PLD_PPDEvent;

typedef struct
{
    int DivaScriptStart;
    int PPDStart;
    int EVDStart;
    int DivaScriptLen;
    int PPDLen;
    int EVDLen;
}PLD_PPDFilePosition;

int PLD_AssignNotesImages(PLD_ArrayList* notes)
{
    int type;

    for (int i = 0; i < notes->len; i++)
    {
        PLD_Note* note = PLD_GetNote(notes, i);

        type = note->type;

        if (PLD_IsHoldType(type))
        {
            type -= 4;
        }
        else if (PLD_IsSlideType(type))
        {
            type -= 8;
        }
        else if (PLD_IsChainBitType(type))
        {
            type -= 4;
        }
        else if (PLD_IsChainType(type))
        {
            type -= 11;
        }

        type *= 4;

        note->buttonName = type + note->isMulti;
        note->targetName = type + 2 + note->isMulti;
    }

    return PLD_SUCCESS;
}

PLD_PPDEvent* PLD_PPDGetEvent(PLD_ArrayList* events, int index)
{
    return events->data[index];
}

int PLD_PPDLoadEvents(SDL_IOStream* layerFile, PLD_PPDFilePosition filePosition, PLD_ArrayList* events)
{
    float time;
    char c;
    size_t ret;

    SDL_SeekIO(layerFile, filePosition.EVDStart, SDL_IO_SEEK_SET);
    ret = SDL_ReadIO(layerFile, &c, sizeof(char));
    SDL_SeekIO(layerFile, -1, SDL_IO_SEEK_CUR);

    while (ret != 0)
    {
        PLD_PPDEvent* event = SDL_calloc(1, sizeof(PLD_PPDEvent));
        SDL_ReadIO(layerFile, &time, sizeof(float));
        SDL_ReadIO(layerFile, &event->type, sizeof(char));
        event->time = time * 1000;

        switch (event->type)
        {
            case PLD_PPD_EVENT_CHANGEVOLUME:
                SDL_SeekIO(layerFile, 2, SDL_IO_SEEK_CUR);
                break;

            case PLD_PPD_EVENT_CHANGEBPM:
                SDL_ReadIO(layerFile, &event->BPM, sizeof(float));
                break;

            case PLD_PPD_EVENT_CHANGEBPMRAPID:
                SDL_ReadIO(layerFile, &event->BPM, sizeof(float));
                SDL_SeekIO(layerFile, 1, SDL_IO_SEEK_CUR);
                break;

            case PLD_PPD_EVENT_CHANGESOUND:
                SDL_SeekIO(layerFile, 4, SDL_IO_SEEK_CUR);
                break;

            case PLD_PPD_EVENT_CHANGEDISPLAY:
                SDL_SeekIO(layerFile, 1, SDL_IO_SEEK_CUR);
                break;

            case PLD_PPD_EVENT_CHANGEMOVESTATE:
                SDL_ReadIO(layerFile, &event->value, sizeof(char));
                break;

            case PLD_PPD_EVENT_CHANGERELEASESOUND:
                SDL_SeekIO(layerFile, 2, SDL_IO_SEEK_CUR);
                break;

            case PLD_PPD_EVENT_CHANGENOTETYPE:
                SDL_SeekIO(layerFile, 1, SDL_IO_SEEK_CUR);
                break;

            case PLD_PPD_EVENT_CHANGEINITIALIZEORDER:
                SDL_SeekIO(layerFile, 4, SDL_IO_SEEK_CUR);
                break;

            case PLD_PPD_EVENT_CHANGESLIDESCALE:
                SDL_ReadIO(layerFile, &event->sliderScale, sizeof(float));
                break;
        }

        PLD_ArrayListAdd(events, event);
        ret = SDL_ReadIO(layerFile, &c, sizeof(char));
        SDL_SeekIO(layerFile, -1, SDL_IO_SEEK_CUR);
    }

    return PLD_SUCCESS;
}

int PLD_IsPPDHoldType(int type)
{
    if (type >= PLD_PPD_TYPE_ISID)
    {
        type -= PLD_PPD_TYPE_ISID;
    }

    return (type == PLD_PPD_TYPE_TRIANGLEHOLD) || (type == PLD_PPD_TYPE_SQUAREHOLD) || (type == PLD_PPD_TYPE_CROSSHOLD) || (type == PLD_PPD_TYPE_CIRCLEHOLD);
}

int PLD_IsPPDSlideType(int type)
{
    if (type >= PLD_PPD_TYPE_ISID)
    {
        type -= PLD_PPD_TYPE_ISID;
    }

    return (type == PLD_PPD_TYPE_SLIDEL) || (type == PLD_PPD_TYPE_SLIDER);
}

int PLD_IsPPDChainType(int type)
{
    if (type >= PLD_PPD_TYPE_ISID)
    {
        type -= PLD_PPD_TYPE_ISID;
    }

    return (type == PLD_PPD_TYPE_CHAINL) || (type == PLD_PPD_TYPE_CHAINR);
}

void PLD_PPDAngleToDegree(float* angle)
{
    *angle *= (float)180/SDL_PI_F;
    *angle *= -1;
    *angle += 90;
    *angle = (int)*angle % 360;
}

void PLD_PPDFixPos(float* posx, float* posy)
{
    *posx *= (float)PLD_LOGICAL_WIDTH / (float)PLD_PPD_WIDTH;
    *posy *= (float)PLD_LOGICAL_HEIGHT / (float)PLD_PPD_HEIGHT;
}

void PLD_PPDConvertType(int* type)
{
    switch (*type)
    {
        case PLD_PPD_TYPE_TRIANGLE:
            *type = PLD_TYPE_TRIANGLE;
            break;

        case PLD_PPD_TYPE_SQUARE:
            *type = PLD_TYPE_SQUARE;
            break;

        case PLD_PPD_TYPE_CROSS:
            *type = PLD_TYPE_CROSS;
            break;

        case PLD_PPD_TYPE_CIRCLE:
            *type = PLD_TYPE_CIRCLE;
            break;

        case PLD_PPD_TYPE_TRIANGLEHOLD:
            *type = PLD_TYPE_TRIANGLEHOLD;
            break;

        case PLD_PPD_TYPE_SQUAREHOLD:
            *type = PLD_TYPE_SQUAREHOLD;
            break;

        case PLD_PPD_TYPE_CROSSHOLD:
            *type = PLD_TYPE_CROSSHOLD;
            break;

        case PLD_PPD_TYPE_CIRCLEHOLD:
            *type = PLD_TYPE_CIRCLEHOLD;
            break;

        case PLD_PPD_TYPE_SLIDEL:
            *type = PLD_TYPE_SLIDEL;
            break;
            
        case PLD_PPD_TYPE_SLIDER:
            *type = PLD_TYPE_SLIDER;
            break;

        case PLD_PPD_TYPE_CHAINL:
            *type = PLD_TYPE_CHAINL;
            break;

        case PLD_PPD_TYPE_CHAINR:
            *type = PLD_TYPE_CHAINR;
            break;

        default:
            *type = PLD_TYPE_TRIANGLE;
            break;
    }
}

int PLD_PPDGetFilePosition(SDL_IOStream* layerFile, PLD_PPDFilePosition* filePosition)
{
    int len, pos = 0;
    char* lenNames = NULL;

    SDL_SeekIO(layerFile, 9, SDL_IO_SEEK_SET);

    PLD_ExtractSequence(layerFile, &lenNames, "\0", 1);

    for (int i = 0; i < SDL_strlen(lenNames); i++)
    {
        char* name = SDL_calloc(lenNames[i] + 1, sizeof(char));

        for (int j = 0; j < lenNames[i]; j++)
        {
            SDL_ReadIO(layerFile, &name[j], sizeof(char));
        }

        SDL_ReadIO(layerFile, &len, sizeof(int));

        if (SDL_strcmp(name, "ppd") == 0)
        {
            filePosition->PPDStart = pos;
            filePosition->PPDLen = len;
        }
        else if (SDL_strcmp(name, "evd") == 0)
        {
            filePosition->EVDStart = pos;
            filePosition->EVDLen = len;
        }
        else if (SDL_strcmp(name, "Scripts\\DivaScript") == 0)
        {
            filePosition->DivaScriptStart = pos;
            filePosition->DivaScriptLen = len;
        }

        pos += len;

        SDL_free(name);
    }

    if (filePosition->PPDStart != 0)
    {
        filePosition->PPDStart += SDL_TellIO(layerFile);
    }
    if (filePosition->EVDStart != 0)
    {
        filePosition->EVDStart += SDL_TellIO(layerFile);
    }
    if (filePosition->DivaScriptStart != 0)
    {
        filePosition->DivaScriptStart += SDL_TellIO(layerFile);
    }

    SDL_free(lenNames);

    return PLD_SUCCESS;
}

int PLD_PPDInsertParam(PLD_ArrayList* notes, char** keys, char** values, int len, int ID)
{
    for (int i = 0; i < notes->len; i++)
    {
        if (PLD_GetNote(notes, i)->ID == ID)
        {
            for (int j = 0; j < len; j++)
            {
                PLD_Note* note = PLD_GetNote(notes, i);

                if (SDL_strcmp(keys[j], "Distance") == 0)
                {
                    note->distance = SDL_atof(values[j]) / 250;
                }
                else if (SDL_strcmp(keys[j], "Amplitude") == 0)
                {
                    note->amplitude = SDL_atof(values[j]);
                }
                else if (SDL_strcmp(keys[j], "Frequency") == 0)
                {
                    float frequency = SDL_atof(values[j]);

                    frequency *= -1;

                    note->frequency = frequency;
                }
                else if (SDL_strcmp(keys[j], "#RightRotation") == 0)
                {
                    note->frequency *= -1;
                }
                else if (SDL_strcmp(keys[j], "FlyingTime") == 0)
                {
                	note->flyingTime = SDL_atof(values[j]);
                }
            }
        }
    }

    return PLD_SUCCESS;
}

int PLD_PPDGetParam(SDL_IOStream* layerFile,  PLD_ArrayList* notes)
{
    int paramLen;

    SDL_SeekIO(layerFile, (int)(sizeof(float)/sizeof(char)), SDL_IO_SEEK_CUR);
    SDL_ReadIO(layerFile, &paramLen, sizeof(int));

    for (int i = 0; i < paramLen; i++)
    {
        int paramID = 0, paramSize = 0, isKey = 0, isValue = 0, second = 0, stringLen = 0;
        char c;

        SDL_ReadIO(layerFile, &paramID, sizeof(int));
        SDL_ReadIO(layerFile, &paramSize, sizeof(int));

        char** keys = SDL_calloc(paramSize, sizeof(char*));
        char** values = SDL_calloc(paramSize, sizeof(char*));

        for (int j = 0; j < paramSize; j++)
        {
            keys[j] = SDL_calloc(paramSize, sizeof(char));
            values[j] = SDL_calloc(paramSize, sizeof(char));
        }

        for (int j = 0; j < paramSize; j++)
        {
            SDL_ReadIO(layerFile, &c, sizeof(char));

            if ((c == '\"') && (!isKey) && (!isValue) && (!second))
            {
                isKey = 1;
            }
            else if ((c == '\"') && (isKey))
            {
                isKey = 0;
                second = 1;
            }
            else if ((c == '\"') && (!isKey) && (!isValue) && (second))
            {
                isValue = 1;
            }
            else if ((c != '\"') && (isKey))
            {
                keys[stringLen][SDL_strlen(keys[stringLen])] = c;
            }
            else if ((c != '\"') && (isValue))
            {
                values[stringLen][SDL_strlen(values[stringLen])] = c;
            }
            else if ((c == '\"') && (!isKey) && (isValue) && (second))
            {
                isValue = 0;
                second = 0;
                stringLen++;
            }
        }

        PLD_PPDInsertParam(notes, keys, values, stringLen, paramID);

        for (int j = 0; j < paramSize; j++)
        {
            SDL_free(keys[j]);
            SDL_free(values[j]);
        }
        
        SDL_free(keys);
        SDL_free(values);
    }

    return PLD_SUCCESS;
}

int PLD_PPDInsertChainslide(PLD_ArrayList* notes, PLD_Note* note, float bpm, float sliderScale)
{
    if (PLD_IsChainType(note->type))
    {
        int gap = 1000 * (float)60/(float)180/(float)8;

        gap /= sliderScale;

        PLD_Note* resNote = SDL_calloc(1, sizeof(PLD_Note));
        *resNote = *note;
        
        if (resNote->type == PLD_TYPE_CHAINL)
        {
            resNote->type = 10;
        }
        else
        {
            resNote->type = 11;
        }

        int slidePos[2] = {-36, 36};

        resNote->targetX += slidePos[resNote->type - PLD_TYPE_CHAINLBIT] / 2;

        while(resNote->time + gap < resNote->rtime)
        {
            resNote->time += gap;

            resNote->targetX += slidePos[resNote->type - PLD_TYPE_CHAINLBIT];

            PLD_Note* dump = SDL_calloc(1, sizeof(PLD_Note));
            *dump = *resNote;
            
            PLD_AddNoteOrdered(notes, dump);
        }

        SDL_free(resNote);
    }

    return PLD_SUCCESS;
}

int PLD_PPDLinkChainslide(PLD_ArrayList* notes)
{
    int i;
    PLD_Note* nextLNote = NULL;
    PLD_Note* nextRNote = NULL;

    for (i=notes->len-1; i>=0; i--)
    {
        PLD_Note* tempNote = PLD_GetNote(notes, i);
        if (tempNote->type == PLD_TYPE_CHAINLBIT)
        {
            if (nextLNote != NULL)
            {
                tempNote->nextChainBit = nextLNote;
            }

            nextLNote = tempNote;
        }
        else if (tempNote->type == PLD_TYPE_CHAINRBIT)
        {
            if (nextRNote != NULL)
            {
                tempNote->nextChainBit = nextRNote;
                nextRNote = tempNote;
            }
            else
            {
                nextRNote = tempNote;
            }
        }
        else if (tempNote->type == PLD_TYPE_CHAINL)
        {
            tempNote->nextChainBit = nextLNote;
            nextLNote = NULL;
        }
        else if (tempNote->type == PLD_TYPE_CHAINR)
        {
            tempNote->nextChainBit = nextRNote;
            nextRNote = NULL;
        }
    }

    return PLD_SUCCESS;
}

PLD_Note* PLD_PPDNoteFromFile(SDL_IOStream* layerFile)
{
    PLD_Note* note = SDL_calloc(1, sizeof(PLD_Note));
    float time, rtime, posx, posy, angle;

    SDL_ReadIO(layerFile, &time, sizeof(float));
    SDL_ReadIO(layerFile, &posx, sizeof(float));
    SDL_ReadIO(layerFile, &posy, sizeof(float));
    SDL_ReadIO(layerFile, &angle, sizeof(float));

    SDL_ReadIO(layerFile, &note->type, sizeof(char));

    if ((PLD_IsPPDHoldType(note->type)) || (PLD_IsPPDChainType(note->type)))
    {
        SDL_ReadIO(layerFile, &rtime, sizeof(float));
    } 

    if (note->type >= PLD_PPD_TYPE_ISID)
    {
        SDL_ReadIO(layerFile, &note->ID, sizeof(int));
        note->type -= PLD_PPD_TYPE_ISID;
    }

    note->distance = 300000 / 250;
    note->amplitude = 500;
    note->frequency = -2;

    PLD_PPDFixPos(&posx, &posy);
    PLD_PPDAngleToDegree(&angle);

    note->time = time * 1000;

    note->targetX = posx;
    note->targetY = posy;

    note->angle = (int)angle;
    note->rtime = rtime * 1000;
    note->active = 1;

    return note;
}

void PLD_SwapMulti(double angle[4], PLD_Note* multi[4], int i, int j)
{
    double tempAngle = angle[i];
    angle[i] = angle[j];
    angle[j] = tempAngle;

    PLD_Note* tempNote = multi[i];
    multi[i] = multi[j];
    multi[j] = tempNote;
}

int PLD_SortMulti(PLD_Note* multi[4]) {
    int centerX;
    int centerY;
    double angle[4];

    centerX = 0;
    centerY = 0;

    for (int i = 0; i < 4; i++)
    {
        centerX += multi[i]->targetX;
        centerY += multi[i]->targetY;
    }

    centerX /= 4;
    centerY /= 4;

    for (int i = 0; i < 4; i++)
    {
        angle[i] = SDL_atan2(multi[i]->targetY - centerY, multi[i]->targetX - centerX);
    }

    if(angle[0] > angle[1]) PLD_SwapMulti(angle, multi, 0, 1);
    if(angle[2] > angle[3]) PLD_SwapMulti(angle, multi, 2, 3);
    if(angle[0] > angle[2]) PLD_SwapMulti(angle, multi, 0, 2);
    if(angle[1] > angle[3]) PLD_SwapMulti(angle, multi, 1, 3);
    if(angle[1] > angle[2]) PLD_SwapMulti(angle, multi, 1, 2);

    return PLD_SUCCESS;
}

int PLD_LinkMulti(PLD_Note* multi[4], int count)
{
    switch (count)
    {
        case 2:
            multi[0]->connect = multi[1];
            break;

        case 3:
            multi[0]->connect = multi[1];
            multi[1]->connect = multi[2];
            multi[2]->connect = multi[0];
            break;

        case 4:
            PLD_SortMulti(multi);

            multi[0]->connect = multi[1];
            multi[1]->connect = multi[2];
            multi[2]->connect = multi[3];
            multi[3]->connect = multi[0];
            break;

        default:
            break;
    }

    return PLD_SUCCESS;
}

int PLD_PPDLoadNotes(SDL_IOStream* layerFile, PLD_Song* songInfo, PLD_ArrayList* events, PLD_PPDFilePosition filePosition)
{
    int multiCount = 0;
    float FLAG = 0, sliderScale = 1, bpm = songInfo->dataIni->bpm;
    int isSlideL = 0, isSlideR = 0, eventIndex = 0;
    PLD_Note* previousNote = NULL;
    PLD_Note* multi[4] = {0};
    
    SDL_SeekIO(layerFile, filePosition.PPDStart + 3, SDL_IO_SEEK_SET);

    while ((FLAG == FLAG) && (SDL_TellIO(layerFile) < filePosition.EVDStart))
    {
        PLD_Note* note = PLD_PPDNoteFromFile(layerFile);

        if (note->type == PLD_PPD_TYPE_SLIDER)
        {
            isSlideR = 1;
        }
        else if (note->type == PLD_PPD_TYPE_SLIDEL)
        {
            isSlideL = 1;
        }
        else if ((previousNote != NULL) && (previousNote->time == note->time))
        {
            if ((isSlideR) && (note->type == PLD_PPD_TYPE_CIRCLE))
            {
                note->type = PLD_PPD_TYPE_SLIDER;
            }
            else if ((isSlideL) && (note->type == PLD_PPD_TYPE_TRIANGLE))
            {
                note->type = PLD_PPD_TYPE_SLIDEL;
            }
        }
        else
        {
            isSlideL = 0;
            isSlideR = 0;
        }
		
        if (previousNote != NULL)
        {
            if (previousNote->time == note->time)
            {
                previousNote->distance = note->distance = 200000 / 250;
                previousNote->frequency = note->frequency = 0;
                previousNote->isMulti = note->isMulti = true;
            
                if (multiCount < 3)
                {
                    multi[multiCount] = previousNote;
                }
                multiCount++;
            }
            else
            {
                if ((multiCount > 0) && (multiCount <= 3))
                {
                    multi[multiCount] = previousNote;
                    multiCount++;
                    PLD_LinkMulti(multi, multiCount);
                }

                multiCount = 0;
            }
        }
		
        while ((eventIndex < events->len) && (PLD_PPDGetEvent(events, eventIndex)->time < note->time))
        {
            PLD_PPDEvent* event = PLD_PPDGetEvent(events, eventIndex);

            if (event->type == PLD_PPD_EVENT_CHANGESLIDESCALE)
            {
                sliderScale = event->sliderScale;
            }

            eventIndex++;
        }
		
        note->flyingTime = 60 * 4000 / bpm;

        PLD_PPDConvertType(&note->type);
		
        PLD_PPDInsertChainslide(songInfo->notes, note, bpm, sliderScale);

		PLD_AddNoteOrdered(songInfo->notes, note);

        previousNote = note;

        SDL_ReadIO(layerFile, &FLAG, sizeof(float));
        SDL_SeekIO(layerFile, (int)(sizeof(float)/sizeof(char)) * -1, SDL_IO_SEEK_CUR);
	}

    if ((multiCount > 0) && (multiCount <= 3))
    {
        multi[multiCount] = PLD_GetNote(songInfo->notes, songInfo->notes->len - 1);
        multiCount++;
        PLD_LinkMulti(multi, multiCount);
    }

    multiCount = 0;

    PLD_PPDLinkChainslide(songInfo->notes);

    if (SDL_TellIO(layerFile) < filePosition.EVDStart)
    {
        PLD_PPDGetParam(layerFile, songInfo->notes);
    }

    return 0;
}

int PLD_LoadPPDFiles(PLD_Song* songInfo, SDL_IOStream** layerFile)
{
    if ((*layerFile = SDL_IOFromFile(songInfo->chartPath, "rb")) == NULL)
    	goto error;

    return PLD_SUCCESS;

error:
    SDL_LogError(0, "Failed to load song files");

    if (*layerFile)
    {
    	SDL_CloseIO(*layerFile);
    	*layerFile = NULL;
    }

    return PLD_FAILURE;
}

int PLD_LoadPPDChart(PLD_Song* songInfo)
{
    SDL_IOStream* layerFile = NULL;
    PLD_PPDFilePosition filePosition = {0};
    PLD_ArrayList* events;

    songInfo->notes = PLD_CreateArrayList();
    songInfo->currentNotes = PLD_CreateArrayList();
    events = PLD_CreateArrayList();

    if (PLD_LoadPPDFiles(songInfo, &layerFile) != PLD_SUCCESS) {
        goto error;
    }

    if (PLD_PPDGetFilePosition(layerFile, &filePosition) != PLD_SUCCESS) {
        goto error;
    }

    if (PLD_PPDLoadEvents(layerFile, filePosition, events) != PLD_SUCCESS) {
        goto error;
    }

    if (PLD_PPDLoadNotes(layerFile, songInfo, events, filePosition) != PLD_SUCCESS) {
        goto error;
    }

    if (PLD_AssignNotesImages(songInfo->notes) != PLD_SUCCESS) {
        goto error;
    }
    
    PLD_DestroyArrayList(events, SDL_free);
    SDL_CloseIO(layerFile);

    return PLD_SUCCESS;

error:
    if (layerFile != NULL) {
        SDL_CloseIO(layerFile);
    }

    if (songInfo->notes != NULL) {
        PLD_DestroyArrayList(songInfo->notes, SDL_free);
    }

    if (songInfo->currentNotes != NULL) {
        PLD_DestroyArrayList(songInfo->notes, NULL);
    }

    if (events != NULL) {
        PLD_DestroyArrayList(events, SDL_free);
    }

    return PLD_FAILURE;
}
