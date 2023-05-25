#include <stdio.h>
#include <stdlib.h>

typedef enum {
    NOTE_C, NOTE_Cs, NOTE_D, NOTE_Eb, NOTE_E, NOTE_F, NOTE_Fs, NOTE_G, NOTE_Gs, NOTE_A, NOTE_Bb, NOTE_B, NOTE_MAX
} note_t;

note_t * genererGamme(char * gamme,int gammeSize,note_t cle);

int main(int argc, char const *argv[])
{
    char gammeChrom[] = {1,1,1,1,1,1,1,1,1,1,1,1};
    char gammeMaj[] = {2,2,1,2,2,2,1};
    char gammeMin[] = {1,2,1,1,2,1,1};
    char gammePenMaj[] = {2,2,3,2,3};

    printf("%d\n",sizeof(gammeMaj));

    printf("gamme\n");
    // for (size_t i = 0; i < 7; i++)
    // {
    //     printf("%d\n",(int)gammeMaj[i]);
    // }
    
    printf("note\n");
    note_t * noteMajC = genererGamme(gammeMaj,sizeof(gammeMaj),NOTE_C); 
    // for (size_t i = 0; i < 7; i++)
    // {
    //     printf("%d\n",(int)noteMajC[i]);
    // }
    free(noteMajC);

    printf("note\n");
    note_t * noteMajCs = genererGamme(gammeMaj,sizeof(gammeMaj),NOTE_Cs); 
    // for (size_t i = 0; i < 7; i++)
    // {
    //     printf("%d\n",(int)noteMajCs[i]);
    // }
    free(noteMajCs);

    printf("note\n");
    note_t * noteMajA = genererGamme(gammeMaj,sizeof(gammeMaj),NOTE_A); 
    // for (size_t i = 0; i < 7; i++)
    // {
    //     printf("%d\n",(int)noteMajA[i]);
    // }
    free(noteMajA);

    printf("note\n");
    note_t * notePenC = genererGamme(gammePenMaj,sizeof(gammePenMaj),NOTE_C); 
    free(notePenC);

    printf("note a\n");
    note_t * noteChrmC= genererGamme(gammeChrom,sizeof(gammeChrom),NOTE_C); 
    free(noteChrmC);
    return 0;
    
}

note_t * genererGamme(char * gamme,int gammeSize,note_t cle)
{
    int j = 0;
    note_t * res = (note_t *)malloc(gammeSize);
    if(!res)
        printf("erreur allocation\n");
    for (int i = (int)cle; j < gammeSize; i++)
    {
        printf("clé = %d\n",(int)cle);
        res[j] = cle;
        cle = (note_t)(((int)cle + (int)gamme[j])%(int)NOTE_MAX);
        j++;
    }
    return res;
}
