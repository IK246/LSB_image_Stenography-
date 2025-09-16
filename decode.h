#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

typedef struct _DecodeInfo // Structure to store information required for decoding
{
    char *stego_image_fname; // stego image file name
    FILE *fptr_stego_image; // stego image file pointer

    char secret_fname[20]; // secret file name
    FILE *fptr_secret; // secret file pointer
    char extn_secret_file[5]; // secret file extension
    char secret_data[10000]; // secret data
    long size_secret_file; // secret file size
    int extn_size; // secret file extension size

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *[], DecodeInfo *);

/* Open all the files */
Status open_files_dec(DecodeInfo *);

/* Decode the magic string */
Status decode_magic_string(const char * , DecodeInfo *);

/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *);

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *);

/* Load secret file data to file */
Status load_data_to_secret_file(DecodeInfo *);

/* Decode byte from the lsb */
char decode_lsb_to_byte(char *);

/* Decode size from the lsb */
int decode_lsb_to_size(char *);

/* Perform the decoding */
Status do_decoding(DecodeInfo *);

#endif
