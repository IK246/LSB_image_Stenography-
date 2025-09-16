/* Header files */
#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *dec_info) // reads and validates the decode arguments
{
    char *ptr; // to store the pointer to the extension
    if((ptr = strstr(argv[2], ".bmp")) != NULL) // checks if the .bmp is present in the argument
    {
        if(!strcmp(ptr, ".bmp")) // checks if the .bmp is present in the end the string
            dec_info->stego_image_fname = argv[2]; // store the argument structure variable
        else
        {
            printf(R "INFO:" Re " Destinaton File extension misplaced (%s)\n", argv[2]);
            return e_failure;
        }
    }
    else
    {
        printf(R "INFO:" Re " Invalid Destination File extension (%s)\n", argv[2]);
        return e_failure;
    }
    if(argv[3] != NULL) // if the secret file name is provided
        strcpy(dec_info->secret_fname, argv[3]); // store the argument structure variable
    else
        strcpy(dec_info->secret_fname, "decoded_secret"); // default name

    return e_success;
}

Status open_files_dec(DecodeInfo *dec_info) // opens the required files
{
    puts(C "INFO:" Re " Opening required files");
    dec_info->fptr_stego_image = fopen(dec_info->stego_image_fname, "r"); // opns the file in r mode
    if(dec_info->fptr_stego_image == NULL) // if file pointer is null
    {
        perror("fopen");
        fprintf(stderr, R "ERROR:" Re " Unable to open file %s\n", dec_info->stego_image_fname); // print the error
        return e_failure;
    }
    else
        printf(G "INFO:" Re " Opened %s\n", dec_info->stego_image_fname);
    return e_success;
}

Status decode_magic_string(const char *magic_string, DecodeInfo *dec_info) // decodes the magic string from the stego image
{
    int i=0;
    puts(C "INFO:" Re " Decoding Magic String Signature");
    fseek(dec_info->fptr_stego_image, 54, SEEK_SET); // moves the file pointer to the 54th byte
    char image_buffer[8]; // buffer to store 8 bytes of image
    while(magic_string[i]) // loops for the length of the magic string
    {
        fread(image_buffer, 8, 1, dec_info->fptr_stego_image); // reads 8 bytes from the stego image
        char ch = decode_lsb_to_byte(image_buffer); // decodes the byte from lsb
        if(ch != magic_string[i]) // checks if the decoded byte is equal to the magic string byte
            return e_failure;
        i++;
    }
    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *dec_info) // decodes the size of the extension of the secret file
{
    puts(C "INFO:" Re " Decoding Secret File Extension Size");
    char image_buffer[32]; // buffer to store 32 bytes of image
    fread(image_buffer, 32, 1, dec_info->fptr_stego_image); // reads 32 bytes from the stego image
    dec_info->extn_size = decode_lsb_to_size(image_buffer); // decodes the size from lsb
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *dec_info) // decodes the extension of the secret file
{
    puts(C "INFO:" Re " Decoding Secret File Extension"); 
    char image_buffer[8]; // buffer to store 8 bytes of image
    for(int i=0; i<dec_info->extn_size; i++) // loops for the size of the extension
    {
        fread(image_buffer, 8, 1, dec_info->fptr_stego_image); // reads 8 bytes from the stego image
        dec_info->extn_secret_file[i] = decode_lsb_to_byte(image_buffer); // decodes the byte from lsb
    }
    dec_info->extn_secret_file[dec_info->extn_size] = '\0'; // null terminate the string
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *dec_info) // decodes the size of the secret file
{
    puts(C "INFO:" Re " Decoding Secret File Size");
    char image_buffer[32]; // buffer to store 32 bytes of image
    fread(image_buffer, 32, 1, dec_info->fptr_stego_image); // reads 32 bytes from the stego image
    dec_info->size_secret_file = decode_lsb_to_size(image_buffer); // decodes the size from lsb
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *dec_info) // decodes the data of the secret file
{
    puts(C "INFO:" Re " Decoding Secret File Data");
    char image_buffer[8]; // buffer to store 8 bytes of image
    for(int i=0; i<dec_info->size_secret_file; i++) // loops for the size of the secret file
    {
        fread(image_buffer, 8, 1, dec_info->fptr_stego_image); // reads 8 bytes from the stego image
        dec_info->secret_data[i] = decode_lsb_to_byte(image_buffer); // decodes the byte from lsb
    }
    dec_info->secret_data[dec_info->size_secret_file] = '\0'; // null terminate the string
    return e_success;
}

Status load_data_to_secret_file(DecodeInfo *dec_info) // loads the decoded data to the secret file
{
    printf(C "INFO:" Re " Loading Secret Data to %s File\n", dec_info->secret_fname);
    fwrite(dec_info->secret_data, dec_info->size_secret_file, 1, dec_info->fptr_secret); // writes the data to the secret file
    return e_success;
}

char decode_lsb_to_byte(char *image_buffer) // decodes a byte from the lsb of the image buffer
{
    char data = 0; // to store the decoded byte
    for(int i=7; i>=0; i--) // loops for 8 bits of the byte
        data |= (image_buffer[7-i] & 1) << i; // decodes the bit from lsb
    return data;
}

int decode_lsb_to_size(char *image_buffer) // decodes a size from the lsb of the image buffer
{
    int size = 0; // to store the decoded size
    for(int i=31; i>=0; i--) // loops for 32 bits of the size
        size |= (image_buffer[31-i] & 1) << i; // decodes the bit from lsb
    return size;
}

Status do_decoding(DecodeInfo *dec_info) // do the decoding
{
    int i = 0, extn_size;
    char *ptr;
    if(open_files_dec(dec_info) == e_failure) // opens the files
        return e_failure;
    else
        puts(G "INFO:" Re " Done");
    puts(C "INFO:" Re " ## Decoding Procedure Started ##");
    if(decode_magic_string(MAGIC_STRING, dec_info) == e_failure) // decodes the magic string from the stego image
    {
        puts(R "INFO:" Re " Magic String not prsent");
        return e_failure;
    }
    else
        puts(G "INFO:" Re " Done");
#if 1
    if(decode_secret_file_extn_size(dec_info) == e_failure) // decodes the size of the extension of the secret file
    {
        puts(R "INFO:" Re " ERROR in Decoding Secret File Extension Size");
        return e_failure;
    }
    else
        puts(G "INFO:" Re " Done");
    if(decode_secret_file_extn(dec_info) == e_failure) // decodes the extension of the secret file
    {
        puts(R "INFO:" Re " ERROR in Decoding Secret File Extension");
        return e_failure;
    }
    else
        puts(G "INFO:" Re " Done");
    if((ptr = strstr(dec_info->secret_fname, ".")) != NULL) // checks if the . is present in the secret file name
    {
        if(!strcmp(ptr, dec_info->extn_secret_file)) // checks if the extension is present in the end the string
        {
open:
            if(!(dec_info->fptr_secret = fopen(dec_info->secret_fname, "w"))) // opens the secret file in w mode
            {
                perror("fopen");
                fprintf(stderr, R "ERROR:" Re " Unable to open file %s\n", dec_info->secret_fname); // print the error
                return e_failure;
            }
            else
                puts(G "INFO:" Re " Opened Secret File");
        }
        else
        {
            *ptr = '\0'; // null terminate the string at the position of .
            goto cat;
        }
    }
    else
    {
cat:
        strcat(dec_info->secret_fname, dec_info->extn_secret_file); // concatenate the extension to the secret file name
        goto open;
    }
    if(decode_secret_file_size(dec_info) == e_failure) // decodes the size of the secret file
    {
        puts(R "INFO:" Re " ERROR in Decoding Secret File Size");
        return e_failure;
    }
    else
        puts(G "INFO:" Re " Done");
    if(decode_secret_file_data(dec_info) == e_failure) // decodes the data of the secret file
    {
        puts(R "INFO:" Re " ERROR in Decoding Secret File Data");
        return e_failure;
    }
    else
        puts(G "INFO:" Re " Done");
    if(load_data_to_secret_file(dec_info) == e_failure) // loads the decoded data to the secret file
    {
        puts(R "INFO:" Re " ERROR in loading data to file");
        return e_failure;
    }
    else
        puts(G "INFO:" Re " Done");

#endif
    return e_success; // return success
}
