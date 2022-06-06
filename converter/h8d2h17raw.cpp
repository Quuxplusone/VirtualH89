// 
// Convert H8D images into h17raw*** images.
// Currently, only handles single-sided/48 tpi disk images
//
// ***Temporary program/format, since h17raw does not explicitly includes number
//    of sides, or number of tracks per side, this has some of the same problems
//    as H8D. 
//
// \todo - objectify program/remove globals.
//       - generate h17disk images, not h17raw.
//
// Mark Garlanger - http://heathkit.garlanger.com
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool hdos;

typedef unsigned char BYTE;

BYTE rawImage[40][3200];

BYTE image[102400];
BYTE checkSum_m;
BYTE volume_m;

const BYTE PrefixSyncChar = 0xfd;

void updateCheckSum(BYTE val)
{
    // First XOR it.
    checkSum_m ^= val;

    // Now rotate left.
    checkSum_m = (checkSum_m >> 7) | (checkSum_m << 1);
}


void writeByte(BYTE track, unsigned pos, BYTE val)
{
    rawImage[track][pos] = val;
    updateCheckSum(val);
}

void printUsage(char *name)
{
    printf("Usage:\n");
    printf(" convert hdos h8d file to h17raw format:\n");
    printf("    %s <h8d file> <h17raw file>\n", name);
    printf(" convert cpm h8d file to h17raw format:\n");
    printf("    %s -c <h8d file> <h17raw file>\n", name);
    exit(1);
}

bool processDisk(void)
{
    unsigned int pos;
    BYTE vol;

    if (hdos)
    {
        printf("Disk Vol: %d\n", volume_m);
    }

    // Process each track
    for (int track = 0; track < 40; track++)
    {
        vol = (track > 0) ? volume_m : 0;;
        
        pos = 0;
        // process each sector
        for(int sector = 0; sector < 10; sector++)
        {
            // initial padding
            for(int padding = 0; padding < 20; padding++)
            {
                rawImage[track][pos++] = 0;
            }

            // header sync byte
            rawImage[track][pos++] = PrefixSyncChar;

            checkSum_m = 0;

            // Volume number
            writeByte(track, pos++, vol);

            // Track number
            writeByte(track, pos++, track);

            // Sector number
            writeByte(track, pos++, sector);

            // Header checksum
            rawImage[track][pos++] = checkSum_m;

            // padding between header and data block
            for(int padding = 0; padding < 10; padding++)
            {
                rawImage[track][pos++] = 0;
            }

            // data block sync byte
            rawImage[track][pos++] = PrefixSyncChar;

            checkSum_m = 0;

            // determine position in H8D input file
            int diskPos = (track * 10 + sector) * 256;

            // write all 256 bytes of the data block
            for(int sec_pos = 0; sec_pos < 256; sec_pos++)
            {
                writeByte(track, pos++, image[diskPos++]);
            }

            // Checksum for the data block
            rawImage[track][pos++] = checkSum_m;

            // additional padding at the end
            for(int padding = 0; padding < 27; padding++)
            {
                rawImage[track][pos++] = 0;
            }
        }
    }

    return(true);
}

bool loadH8D(const char *name)
{
    FILE *file;

    bool pass = true;

    if ((file = fopen(name, "r")) != NULL)
    {
        int num;
        if ((num = fread(image, 256, 10*40, file )) != 400)
        {
            printf("%s: read failed %s - %d\n", __FUNCTION__, name, num);
            pass = false;
        }

        fclose(file);
    }
    else
    {
        printf("%s: unable to open file - %s\n", __FUNCTION__, name);
        pass = false; 
    }

    if (pass) 
    {
        if (hdos)
        {
            volume_m = image[9*256];
            printf("Loaded H8D HDOS - Vol = %d", volume_m);
        }
        else
        {
            volume_m = 0;
            printf("Loaded H8D CP/M");

        }
        printf("(%s): Success\n", name);
    }

    return(pass);
}

void saveH17Disk(const char *name)
{
    FILE *file;

    if ((file = fopen(name, "w")) != NULL)
    {
        int num;
        if ((num = fwrite(rawImage, 320, 10*40, file )) != 400)
        {
            printf("%s: write failed %s - %d\n", __FUNCTION__, name, num);
        }

        fclose(file);
        printf("Save h17raw(%s): Success\n", name);
    }
    else
    {
        printf("%s: unable to open file - %s\n", __FUNCTION__, name);
    }
}

int main(int argc, char *argv[])
{
    char *in_name, *out_name;

    // assume HDOS
    hdos = true;

    // TODO - properly handle options. 
    //      - allow both -h (hdos) and -c (cp/m)
    //      - allow -2 (double-sided)
    //      - allow -4 (48 tpi)
    //      - allow -9 (96 tpi)
    //   - more validation
 
    if (argc == 4)
    {
        if(strncmp("-c",argv[1], 2) == 0)
        {
            hdos = false;
        }
        in_name = argv[2];
        out_name = argv[3]; 
    }
    else if (argc == 3)
    {
        in_name = argv[1];
        out_name = argv[2]; 
    }
    else
    {
        printUsage(argv[0]);
    }

    if (hdos)
    {
        printf("HDOS - Conversion\n");
    }
    else
    {
        printf("CP/M - Conversion\n");
    }

    if (loadH8D(in_name))
    {
        processDisk();
        saveH17Disk(out_name);
    }

    return(0);
}

