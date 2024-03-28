/*
This script constitutes an integral component of the Wave Player Challenge, orchestrated by taksuntech, designed to operate on
the Zynq (SoC). In its first version, the primary objective is to read a WAV file data from PS, allocate this data to DDR memory,
and subsequently transmit it via the AXI bus to the PWM block within the PL.

The development of this code is aimed at reading a WAV file from an SD card. It meticulously parses the header file information
into a structure named "waveFileHeaderStructure" and then proceeds to relay this header information to the terminal
by "PrintHeaderFile" function. Following this, the script embarks on reading the data portion of the file by "ReadWavFile_Data" function.

This script has been developed in c++ by Ali Kh in March 20, 2024.

For additional information, please refer to:
https://taksuntech.ir/2024/02/26/zynq-challenge1-wave-player/
https://github.com/alghasi/Wave_Reader
*/
#include "xsdps.h"
#include "xil_printf.h"
#include "ff.h"

#include "xparameters.h"
#include "xil_io.h"
#include "sleep.h"

#include <stdio.h>
const size_t WAV_HEADER_SIZE = 44;

static FATFS fatfs;


static char *SD_File;
typedef struct {
    unsigned char ChunkID [4]   ;
    int ChunkSize               ;
    unsigned char Format[4]     ;
    unsigned char SubChunk1ID[4];
    int SubChunk1Size           ;
    short AudioFormat           ;
    short NumChannels           ;
    int SampleRate              ;
    int ByteRate                ;
    short BlockAlign            ;
    short BitsPerSamples        ;
    unsigned char SubChunk2ID[4];
    int SubChunk2Size           ;
}waveFileHeaderStructure;



int ReadWavFile_Header(unsigned char* WavFileName,unsigned char* header_data, size_t size)
{
    FRESULT Res;
    UINT NumBytesRead;
    TCHAR *Path = "0:/";
    Res = f_mount(&fatfs, Path, 0);
    if (Res != FR_OK) { return XST_FAILURE; }
    SD_File = (char *)WavFileName;
    FIL file;
    Res = f_open(&file, SD_File, FA_READ);
    if (Res) { return XST_FAILURE; }
    Res = f_lseek(&file, 0);
    if (Res) { return XST_FAILURE; }
    Res = f_read(&file, (void*)header_data, size, &NumBytesRead);
    if (Res) { return XST_FAILURE; }
    Res = f_close(&file);
    if (Res) { return XST_FAILURE; }
    return XST_SUCCESS;
}
int ReadWavFile_Data(unsigned char* WavFileName, unsigned char* data1, size_t size)//file name
{
	FRESULT Res;
    UINT NumBytesRead;
    TCHAR *Path = "0:/";
    Res = f_mount(&fatfs, Path, 0);
    if (Res != FR_OK) { return XST_FAILURE; }
    SD_File = (char *)WavFileName;
    FIL file;
    Res = f_open(&file, SD_File, FA_READ);
    if (Res) { return XST_FAILURE; }
    Res = f_lseek(&file, 44);
    if (Res) { return XST_FAILURE; }
    Res = f_read(&file, (void*)data1, size, &NumBytesRead);
    xil_printf("NumBytesRead: %d \r\n",NumBytesRead);
    if (Res) { return XST_FAILURE; }
    Res = f_close(&file);
    if (Res) { return XST_FAILURE; }
    return XST_SUCCESS;
}
void HeaderStruct_SetValue(unsigned char *Header_data,waveFileHeaderStructure *Header_struct)
{
	memcpy(&Header_struct->ChunkID        , Header_data + 0 , 4);
	memcpy(&Header_struct->ChunkSize      , Header_data + 4 , 4);
	memcpy(&Header_struct->Format         , Header_data + 8 , 4);
	memcpy(&Header_struct->SubChunk1ID    , Header_data + 12, 4);
	memcpy(&Header_struct->SubChunk1Size  , Header_data + 16, 4);
	memcpy(&Header_struct->AudioFormat    , Header_data + 20, 2);
	memcpy(&Header_struct->NumChannels    , Header_data + 22, 2);
	memcpy(&Header_struct->SampleRate     , Header_data + 24, 4);
	memcpy(&Header_struct->ByteRate       , Header_data + 28, 4);
	memcpy(&Header_struct->BlockAlign     , Header_data + 32, 2);
	memcpy(&Header_struct->BitsPerSamples , Header_data + 34, 2);
	memcpy(&Header_struct->SubChunk2ID    , Header_data + 36, 4);
	memcpy(&Header_struct->SubChunk2Size  , Header_data + 40, 4);
}
void PrintHeaderFile(waveFileHeaderStructure header1)
{

    xil_printf("ChunkID: %s \r\n",header1.ChunkID);
    xil_printf("ChunkSize: %d \r\n", header1.ChunkSize);
    xil_printf("Format: %s \r\n",header1.Format);
    xil_printf("SubChunk1ID: %s \r\n", header1.SubChunk1ID);
    xil_printf("SubChunk1Size :%d \r\n", header1.SubChunk1Size);
    xil_printf("AudioFormat: %d \r\n", header1.AudioFormat);
    xil_printf("NumChannels: %d \r\n", header1.NumChannels);
    xil_printf("SampleRate: %d \r\n", header1.SampleRate);
    xil_printf("ByteRate: %d \r\n", header1.ByteRate);
    xil_printf("BlockAlign: %d \r\n", header1.BlockAlign);
    xil_printf("BitsPerSamples: %d \r\n", header1.BitsPerSamples);
    xil_printf("SubChunk2ID: %s \r\n",header1.SubChunk2ID);
    xil_printf("SubChunk2Size: %d \r\n", header1.SubChunk2Size);
}


int main(void)
{
	init_platform();
    int Status;
    xil_printf("Hello! Wave file reader program is started ... \r\n");
    static unsigned char WavFileName[32] = "sine1.wav";

    static unsigned char Header_data[44];
    //read the first 44 bytes of the wav file
    Status = ReadWavFile_Header(WavFileName, Header_data, sizeof(Header_data));
    if (Status != XST_SUCCESS)
    {
   	 xil_printf("reading the file from SD card to extract the header file information failed! Please check the file name or be sure if the wave file exist \r\n");
   	 return XST_FAILURE;
    }
    else
    {
    	xil_printf("The header file read successfully \r\n");
    }

    //create header object
    waveFileHeaderStructure header1;
    //fill the object with data
    HeaderStruct_SetValue(Header_data, &header1);
    //print the object to the serial terminal. Baud rate:115200
    PrintHeaderFile(header1);

    //specify number of bytes of data that you want to read
    size_t ssize=header1.SubChunk2Size; //max 15360
    //create a variable to store the data
    unsigned char data1[ssize];//176400
    //read the fist ssize byte of data
    int result = ReadWavFile_Data(WavFileName, data1, sizeof(data1));
    if (result == XST_SUCCESS)
    {
    	xil_printf("Reading the data was successful \r\n");
    }
    else
    {
        xil_printf("Reading the data was failed \r\n");
    }

    //casting data to two bytes (int) and then send them to the terminal
    for(int i = 0; i < ssize / 2; i++) // Each sample is 2 bytes, so total samples are half the total bytes
    {
        int16_t sample;
        memcpy(&sample, data1 + i*2, sizeof(sample)); // Copy 2 bytes into sample
        xil_printf("data %d:    %d\r\n", i, sample);
        //Xil_Out32(XPAR_PWM_0_S00_AXI_BASEADDR+8,sample);
        //usleep(1);
    }

    xil_printf("Successfully read wav file and showed the header and the data of the file! :-) \r\n");

    cleanup_platform();

    return XST_SUCCESS;
}
