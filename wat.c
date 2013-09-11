#include "wat.h"

WavHeader * wav_header = NULL;

int read_header_file(char *file_name)
{
        FILE *f;        
        long file_size;
        size_t ret;
        unsigned char * buffer;

        f = fopen(file_name, "r");
        if(f == NULL){
                printf("\nFile Error");
        }

        buffer = (unsigned char *) malloc(sizeof(unsigned char *) * HEADER_SIZE);
        if(buffer == NULL){
                printf("\nMemory Error");
        }

        ret = fread(buffer, sizeof(unsigned char *), HEADER_SIZE, f);

        fclose(f);

        if(ret < HEADER_SIZE){
                printf("\nSomething is wrong with fread");
        }

        wav_header->chunk_size = 1234;

        return 1;
}


int main(int argc, char *argv[])
{
        FILE *pFile;
        long file_size;
        unsigned char * buffer;
        size_t result;

        read_header_file("toque.wav");

        pFile = fopen("toque.wav", "r");
        if (pFile == NULL){
                printf("\nFile error\n");
        }

        fseek(pFile, 0, SEEK_END);
        file_size = ftell(pFile); 
        rewind(pFile);

        buffer = (unsigned char *) malloc(sizeof(unsigned char*) * file_size);

        if(buffer == NULL){
                printf("\nMemory error\n");
        }

        result = fread(buffer, sizeof(unsigned char), file_size, pFile);
 
        //tamanho do arquivo wav
        printf("\nsize => %lu bytes\n", file_size);

        int i;
        printf("\nRIFF chunk descriptor (ASC) => ");
        for (i = 0; i < 4; i++){
        }
                 printf("%s", wav_header->chunk_id);

        fseek(pFile, 4, SEEK_SET);
        int size;
        result = fread(&size, 4, 1, pFile);
        printf("\n\nChunk size (int) => ");
        printf("%d", size);


        printf("\n\nFormat (ASC) => ");
        for (i = 8; i < 12; i++){
                 printf("%c", buffer[i]);
        }
        
        fseek(pFile, 12, SEEK_SET);
        result = fread(&size, 4, 1, pFile);
        printf("\n\nChunk size (int) => ");
        //for(i = 12; i < 16; i++){
                printf("%d", size);
        //}

        fseek(pFile, 16, SEEK_SET);
        result = fread(&size, 4, 1, pFile);
        printf("\n\nSubChunk1 size (int) => ");
        printf("%d", size);

        fseek(pFile, 20, SEEK_SET);
        result = fread(&size, 2, 1, pFile);

        printf("\n\nAudio Format (int) => ");
        printf("%d", size);

        fseek(pFile, 22, SEEK_SET);
        result = fread(&size, 2, 1, pFile);

        printf("\n\nNumChannels (int) => ");
        printf("%d", size);

        fseek(pFile, 24, SEEK_SET);
        result = fread(&size, 4, 1, pFile);

        printf("\n\nSample Rate (int) => ");
        printf("%d", size);

        fseek(pFile, 28, SEEK_SET);
        result = fread(&size, 4, 1, pFile);

        printf("\n\nByte Rate (int) => ");
        printf("%d", size);

        fseek(pFile, 32, SEEK_SET);
        result = fread(&size, 2, 1, pFile);

        printf("\n\nBlock Align (int) => ");
        printf("%d", size);

        fseek(pFile, 34, SEEK_SET);
        result = fread(&size, 2, 1, pFile);

        printf("\n\nBits per Sample (int) => ");
        printf("%d", size);

        
        fseek(pFile, 34, SEEK_SET);
        result = fread(&size, 2, 1, pFile);

        printf("\n\nBits per Sample (int) => ");
        printf("%d", size);


        printf("\n\nSubchunk2ID (ASC) => ");
        for (i = 36; i < 40; i++){
                printf("%c", buffer[i]);
        }
 
        fseek(pFile, 40, SEEK_SET);
        result = fread(&size, 4, 1, pFile);

        printf("\n\nSubchunk2Size (int) => ");
        printf("%d", size);

        printf("\n\nChunk Size (hex) => ");
        for (i = 40; i < 44; i++){
                printf(" %02X ", buffer[i]);
        }

        printf("\n\v DATA \v");


        for(i = 44; i < 500; i++){
                if(i % 4 == 0)
                        printf("\n");
                if(i % 2 == 0)
                        printf(".");
                printf(" %02X ", buffer[i]);
        }






 



        printf("\n\nend of program\n");
        fclose(pFile);
        return 1;
}
