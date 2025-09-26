#include <PLD/PLD_dsc.h>

#define PLD_DSC_INTSIZE 4

typedef enum
{
    PLD_DSC_OPCODE_INVALID = -1,
    PLD_DSC_OPCODE_END = 0,
    PLD_DSC_OPCODE_TIME = 1,
    PLD_DSC_OPCODE_TARGET = 6,
    PLD_DSC_OPCODE_START = 25,
    PLD_DSC_OPCODE_BARTIMESET = 28,
    PLD_DSC_OPCODE_FLYINGTIME = 58,
    PLD_DSC_OPCODE_MAX = 107
}PLD_DSCOpcode;

const int PLD_DSC_OPCODE_LEN[PLD_DSC_OPCODE_MAX] = {0, 1, 4, 2, 2, 2, 7, 4, 2, 6, 2, 1, 6, 2, 1, 1, 3, 2, 3, 5, 5, 4, 4, 5, 2, 0, 2, 4, 2, 2, 1, 21, 0, 3, 2, 5, 1, 1, 7, 1, 1, 2, 1, 2, 1, 2, 3, 3, 1, 2, 2, 3, 6, 6, 1, 1, 2, 3, 1, 2, 2, 4, 4, 1, 2, 1, 2, 1, 1, 3, 3, 3, 2, 1, 9, 3, 2, 4, 2, 3, 2, 24, 1, 2, 1, 3, 1, 3, 4, 1, 2, 6, 3, 2, 3, 3, 4, 1, 1, 3, 3, 4, 1, 3, 3, 8, 2};

bool PLD_IsDSCFile(SDL_IOStream* dscFile)
{
    //char header[5] = {33, 9, 5, 20, 0};
    char actualHeader[5] = {0};
    long int position = SDL_TellIO(dscFile);

    SDL_SeekIO(dscFile, 0L, SDL_IO_SEEK_SET);

    SDL_ReadIO(dscFile, actualHeader, sizeof(char) * 4);

    SDL_SeekIO(dscFile, position, SDL_IO_SEEK_SET);
    
    //return !SDL_strncmp(header, actualHeader, 4);
	return true;
}


PLD_DSCOpcode PLD_GetOpcode(SDL_IOStream* dscFile)
{
    PLD_DSCOpcode opcode;

    if (SDL_ReadIO(dscFile, &opcode, PLD_DSC_INTSIZE) != PLD_DSC_INTSIZE) {
        return PLD_DSC_OPCODE_INVALID;
    }

    if (opcode > PLD_DSC_OPCODE_MAX - 1) {
        return PLD_DSC_OPCODE_INVALID;
    }

    return opcode;
}


int PLD_PLDNoteFromDSCNote(PLD_Note* note)
{
	switch (note->type)
	{
		case 0:
			note->type = PLD_TYPE_TRIANGLE;
			break;
		
		case 1:
			note->type = PLD_TYPE_CIRCLE;
			break;
		
		case 2:
			note->type = PLD_TYPE_CROSS;
			break;
		
		case 3:
			note->type = PLD_TYPE_SQUARE;
			break;
		
		case 4:
			note->type = PLD_TYPE_TRIANGLEHOLD;
			break;
		
		case 5:
			note->type = PLD_TYPE_CIRCLEHOLD;
			break;
		
		case 6:
			note->type = PLD_TYPE_CROSSHOLD;
			break;
		
		case 7:
			note->type = PLD_TYPE_SQUAREHOLD;
			break;
			
		case 12:
			note->type = PLD_TYPE_SLIDEL;
			break;
		
		case 13:
			note->type = PLD_TYPE_SLIDER;
			break;
		
		case 15:
			note->type = PLD_TYPE_CHAINL;
			break;
		
		case 16:
			note->type = PLD_TYPE_CHAINR;
			break;
			
		case 18:
			note->type = PLD_TYPE_TRIANGLE;
			break;
		
		case 19:
			note->type = PLD_TYPE_CIRCLE;
			break;
		
		case 20:
			note->type = PLD_TYPE_CROSS;
			break;
		
		case 21:
			note->type = PLD_TYPE_SQUARE;
			break;
		
		case 23:
			note->type = PLD_TYPE_SLIDEL;
			break;
		
		case 24:
			note->type = PLD_TYPE_SLIDER;
			break;
		
		default:
			note->type = PLD_TYPE_TRIANGLE;
			break;
	}
	
	note->angle /= 1000;

	note->distance /= 250;
	note->targetX *= (PLD_LOGICAL_WIDTH / 480000.0);
	note->targetY *= (PLD_LOGICAL_HEIGHT / 270000.0);
	
	return PLD_SUCCESS;
}


int PLD_DelayStart(PLD_Song* songInfo, int time)
{
	PLD_Note* note = NULL;
	
	for (int i = 0; i < songInfo->notes->len; i++)
	{
		note = (PLD_Note*)songInfo->notes->data[i];
		
		note->time -= time;
	}
	
	songInfo->dataIni->start = -time;
	
	return PLD_SUCCESS;
}


int PLD_LoadDSCChart(PLD_Song* songInfo)
{
	int offset = 0;
	int bpm = 0;
	bool foundOffset = false;
	PLD_Note* multi[4] = {0};
	int prevChainLGap = -1;
	int prevChainRGap = -1;
	PLD_Note* prevChainL = NULL;
	PLD_Note* prevChainR = NULL;
	int multiCount = 0;
    int opcode = 0;
    int prevTime = -1;
    int time = -1;
    int flyingTime = 0;
    PLD_Note* note = NULL;
	PLD_Note* prevNote = NULL;
    SDL_IOStream* dscFile = NULL;
    
    if ((dscFile = SDL_IOFromFile(songInfo->chartPath, "rb")) == NULL)
    {
    	SDL_Log("Failed to open DSC file");
    	return PLD_FAILURE;
    }

    if (!PLD_IsDSCFile(dscFile))
    {
        SDL_Log("Wrong header, not a DSC file");
        return PLD_FAILURE;
    }
    
    songInfo->notes = PLD_CreateArrayList();
    songInfo->currentNotes = PLD_CreateArrayList();

    SDL_SeekIO(dscFile, 4L, SDL_IO_SEEK_SET);

    opcode = PLD_GetOpcode(dscFile);
    if (opcode == PLD_DSC_OPCODE_INVALID) {
        goto invalidOpcode;
    }

    while (opcode != PLD_DSC_OPCODE_END)
    {
        switch (opcode)
        {
            case PLD_DSC_OPCODE_TIME:
                SDL_ReadIO(dscFile, &time, PLD_DSC_INTSIZE);
                time /= 100;
                
                if (foundOffset)
                {
                	time += offset;
                }
                
                break;

            case PLD_DSC_OPCODE_FLYINGTIME:
                SDL_ReadIO(dscFile, &flyingTime, PLD_DSC_INTSIZE);

                break;

            case PLD_DSC_OPCODE_TARGET:
                note = (PLD_Note*)SDL_calloc(1, sizeof(PLD_Note));
                note->active = true;
                
                if (prevNote != NULL)
                {
			        if (prevTime == time)
			        {
			        	note->isMulti = prevNote->isMulti = true;
			        	
			        	if (multiCount < 3)
						{	
					   		multi[multiCount] = prevNote;
						}
						multiCount++;
			        }
			        else
			        {
			        	if ((multiCount > 0) && (multiCount <= 3))
			        	{
			        		multi[multiCount] = prevNote;
			        		multiCount++;
			        		PLD_LinkMulti(multi, multiCount);
			        	}
			        	
			        	multiCount = 0;
			        }
			    }

                note->time = time + flyingTime;
                note->flyingTime = flyingTime;

                SDL_ReadIO(dscFile, &note->type, PLD_DSC_INTSIZE);
                SDL_ReadIO(dscFile, &note->targetX, PLD_DSC_INTSIZE);
                SDL_ReadIO(dscFile, &note->targetY, PLD_DSC_INTSIZE);
                SDL_ReadIO(dscFile, &note->angle, PLD_DSC_INTSIZE);
                SDL_ReadIO(dscFile, &note->distance, PLD_DSC_INTSIZE);
                SDL_ReadIO(dscFile, &note->amplitude, PLD_DSC_INTSIZE);
                SDL_ReadIO(dscFile, &note->frequency, PLD_DSC_INTSIZE);

                PLD_PLDNoteFromDSCNote(note);
                
                if (note->type == PLD_TYPE_CHAINL)
                {
                	if (prevChainL == NULL)
                	{
                		prevChainL = note;
                	}
                	else if (prevChainLGap == -1)
                	{
                		note->type = PLD_TYPE_CHAINLBIT;
                		prevChainLGap = SDL_abs(note->time - prevChainL->time);
                		prevChainL = note;
                	}
                	else
                	{
                		int currentGap = SDL_abs(note->time - prevChainL->time);
                		int totalGap = SDL_abs(prevChainLGap - currentGap);
                		
                		if (totalGap > 10)
                		{
                			prevChainLGap = -1;
                			prevChainL = note;
                		}
                		else
                		{
                			note->type = PLD_TYPE_CHAINLBIT;
                			prevChainLGap = currentGap;
                			prevChainL = note;
                		}
                	}
                }
                else if (note->type == PLD_TYPE_CHAINR)
                {
                	if (prevChainR == NULL)
                	{
                		prevChainR = note;
                	}
                	else if (prevChainRGap == -1)
                	{
                		note->type = PLD_TYPE_CHAINRBIT;
                		prevChainRGap = SDL_abs(note->time - prevChainR->time);
                		prevChainR = note;
                	}
                	else
                	{
                		int currentGap = SDL_abs(note->time - prevChainR->time);
                		int totalGap = SDL_abs(prevChainRGap - currentGap);
                		
                		if (totalGap > 10)
                		{
                			prevChainRGap = -1;
                			prevChainR = note;
                		}
                		else
                		{
                			note->type = PLD_TYPE_CHAINRBIT;
                			prevChainRGap = currentGap;
                			prevChainR = note;
                		}
                	}
                }

                PLD_AddNoteOrdered(songInfo->notes, note);
                
                prevNote = note;
                prevTime = time;

                break;

            case PLD_DSC_OPCODE_START:
                PLD_DelayStart(songInfo, time);
                
                foundOffset = true;
                offset = -time;
                prevTime -= time;
                time -= time;

                break;
                
            case PLD_DSC_OPCODE_BARTIMESET:
            	SDL_ReadIO(dscFile, &bpm, PLD_DSC_INTSIZE);
            	SDL_SeekIO(dscFile, PLD_DSC_INTSIZE, SDL_IO_SEEK_CUR);
            	
            	flyingTime = 60 * 4000 / bpm;
            	break;

            default:
                SDL_SeekIO(dscFile, PLD_DSC_OPCODE_LEN[opcode] * PLD_DSC_INTSIZE, SDL_IO_SEEK_CUR);
                break;
        }

        opcode = PLD_GetOpcode(dscFile);
        if (opcode == PLD_DSC_OPCODE_INVALID) {
            goto invalidOpcode;
        }
    }
    
    if ((multiCount > 0) && (multiCount <= 3))
	{
		multi[multiCount] = prevNote;
		multiCount++;
		PLD_LinkMulti(multi, multiCount);
	}
	multiCount = 0;
    
    songInfo->dataIni->end = time;
    
    PLD_PPDLinkChainslide(songInfo->notes);
    PLD_AssignNotesImages(songInfo->notes);
    
    SDL_CloseIO(dscFile);

    return PLD_SUCCESS;

invalidOpcode:
    SDL_Log("Invalid opcode in DSC file");

	if (songInfo->notes)
    	PLD_DestroyArrayList(songInfo->notes, SDL_free);

	if (dscFile)
		SDL_CloseIO(dscFile);

    return PLD_FAILURE;
}
