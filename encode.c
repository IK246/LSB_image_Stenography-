/* Header files */
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    //printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    //printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    uint size = 0;
    while(fgetc(fptr) != EOF) // loops till the EOF reached
        size++;
    fseek(fptr, 0, SEEK_SET); // rewind to the starting of the file
    return size; // returns the size
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *enc_info)
{
    char *ptr, *extn[10] = {".txt", ".c", ".sh", ".csv"};
    int i;

    if(((ptr = strstr(argv[2], ".bmp")) != NULL)) // checks if the .bmp is present in the argument
    {
        if(!strcmp(ptr, ".bmp")) // checks if the .bmp is present in the end the string
            enc_info->src_image_fname = argv[2]; // store the argument structure variable
        else
        {
            printf(R "INFO:" Re " Source File extension misplaced (%s)\n", argv[2]);
            return e_failure;
        }
    }
    else
    {
        printf(R "INFO:" Re " Invalid Source File extension (%s)\n", argv[2]);
        return e_failure;
    }

    for(i=0; i<4; i++)
    {
        if((ptr = strstr(argv[3], extn[i])) != NULL) // check the extension of the secret file
        {
            if(!strcmp(ptr, extn[i])) // checks if extension is present in the end the string
            {
                enc_info->secret_fname = argv[3];  // store the argument structure variable
                strcpy(enc_info->extn_secret_file, ptr); // store the extension of the secret file in structure variable
                break;
            }
            else
            {
                printf(R "INFO:" Re " Secret File extension misplaced (%s)\n", argv[3]);
                return e_failure;
            }
        }
    }
    if(i == 4)
    {
        printf(R "INFO:" Re " Invalid Secret File extension (%s)\n", argv[3]);
        return e_failure;
    }

    if(argv[4] == NULL)
    {
        enc_info->dest_image_fname = "destination.bmp";
        printf(C "INFO:" Re " Output File not mentioned. Creating %s as default\n", enc_info->dest_image_fname);
    }
    else if((ptr = strstr(argv[4], ".bmp")) != NULL) // checks if the .bmp is present in the argument
    {
        if(!strcmp(ptr, ".bmp")) // checks if the .bmp is present in the end the string
            enc_info->dest_image_fname = argv[4]; // store the argument structure variable
        else
        {
            printf(R "INFO:" Re " Destination File extension misplaced (%s)\n", argv[4]);
            return e_failure;
        }
    }
    else
    {
        printf(R "INFO:" Re " Invalid Destination File extension (%s)\n", argv[4]);
        return e_failure;
    }
    return e_success;
}

Status open_files(EncodeInfo *enc_info)
{
    puts(C "INFO:" Re " Opeing required files");
    enc_info->fptr_src_image = fopen(enc_info->src_image_fname, "r"); // opns the file in r mode
    if (enc_info->fptr_src_image == NULL) // validation
    {
        perror("fopen");
        fprintf(stderr, R "ERROR:" Re " Unable to open file %s\n", enc_info->src_image_fname); // print the error

        return e_failure;
    }
    else
        printf(G "INFO:" Re " Opened %s\n", enc_info->src_image_fname); 

    enc_info->fptr_secret = fopen(enc_info->secret_fname, "r"); // opns the file in r mode
    if (enc_info->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, R "ERROR:" Re " Unable to open file %s\n", enc_info->secret_fname);  // print the error

        return e_failure;
    }
    else
        printf(G "INFO:" Re " Opened %s\n", enc_info->secret_fname);

    enc_info->fptr_dest_image = fopen(enc_info->dest_image_fname, "w"); // opns the file in w mode
    if (enc_info->fptr_dest_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, R "ERROR:" Re " Unable to open file %s\n", enc_info->dest_image_fname);  // print the error

        return e_failure;
    }
    else
        printf(G "INFO:" Re " Opened %s\n", enc_info->dest_image_fname);
    return e_success;
}

Status check_capacity(EncodeInfo *enc_info)
{
    enc_info->image_capacity = get_image_size_for_bmp(enc_info->fptr_src_image); // get size of the source file

    printf(C "INFO:" Re " Checking for %s size\n", enc_info->secret_fname);
    if(enc_info->size_secret_file = get_file_size(enc_info->fptr_secret)) // gets the size of the secret file
        puts(G "INFO:" Re " Done. Not Empty");
    else
    {
        puts(R "INFO:" Re " Secret file is empty");
        return e_failure;
    }

    printf(C "INFO:" Re " Checking for %s capacity to handle %s\n", enc_info->src_image_fname, enc_info->secret_fname);
    if(enc_info->image_capacity > (54 + (strlen(MAGIC_STRING) * 8) + 32 + (strlen(enc_info->extn_secret_file) * 8) + 32 + (enc_info->size_secret_file * 8))) // checks if the source file can store the data from the secret file
        return e_success;
    else
    {
        printf(R "INFO:" Re " %s file is not compatiable to encode with %s file\n", enc_info->src_image_fname, enc_info->secret_fname);
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image) // copy the header of the source file into the destinatio file
{
    puts(C "INFO:" Re " Coping Image Header");
    char imagebuffer[54];
    fseek(fptr_src_image, 0, SEEK_SET);
    fread(imagebuffer, 54, 1, fptr_src_image); // gets the header from the source file
    fwrite(imagebuffer, 54, 1, fptr_dest_image); // stores the header to the destination file
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *enc_info) // encodes the magic string into the destination file
{
    puts(C "INFO:" Re " Encoding Magic String Signature");
    char imagebuffer[8]; // buffer to store 8 bytes of image
    for(int i=0; i<strlen(magic_string); i++) // loops for the length of the magic string
    {
        fread(imagebuffer, 8, 1, enc_info->fptr_src_image); // reads 8 bytes from the source image
        if(encode_byte_to_lsb(magic_string[i], imagebuffer) == e_failure) // encodes the byte to lsb
            return e_failure;
        fwrite(imagebuffer, 8, 1, enc_info->fptr_dest_image); // writes the encoded data to the destination image
    }
    return e_success;
}

Status encode_secret_file_extn_size(int size, EncodeInfo *enc_info) // encodes the size of the extension of the secret file
{
    printf(C "INFO:" Re " Encoding %s File Extenstion Size\n", enc_info->secret_fname);
    char imagebuffer[32]; // buffer to store 32 bytes of image
    fread(imagebuffer, 32, 1, enc_info->fptr_src_image); // reads 32 bytes from the source image
    if(encode_size_to_lsb(size, imagebuffer) == e_failure) // encodes the size to lsb
        return e_failure;
    fwrite(imagebuffer, 32, 1, enc_info->fptr_dest_image); // writes the encoded data to the destination image
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *enc_info) // encodes the extension of the secret file
{
    printf(C "INFO:" Re " Encoding %s File Extenstion\n", enc_info->secret_fname);
    char imagebuffer[8]; // buffer to store 8 bytes of image
    for(int i=0; i<strlen(file_extn); i++) // loops for the length of the extension of the secret file
    {
        fread(imagebuffer, 8, 1, enc_info->fptr_src_image); // reads 8 bytes from the source image
        if(encode_byte_to_lsb(file_extn[i], imagebuffer) == e_failure) // encodes the byte to lsb
            return e_failure;
        fwrite(imagebuffer, 8, 1, enc_info->fptr_dest_image); // writes the encoded data to the destination image
    }
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *enc_info) // encodes the size of the secret file
{
    printf(C "INFO:" Re " Encoding %s File Size\n", enc_info->secret_fname);
    char imagebuffer[32]; // buffer to store 32 bytes of image
    fread(imagebuffer, 32, 1, enc_info->fptr_src_image); // reads 32 bytes from the source image
    if(encode_size_to_lsb(file_size, imagebuffer) == e_failure) // encodes the size to lsb
        return e_failure;
    fwrite(imagebuffer, 32, 1, enc_info->fptr_dest_image); // writes the encoded data to the destination image
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *enc_info) // encodes the data of the secret file
{
    printf(C "INFO:" Re " Encoding %s File Data\n", enc_info->secret_fname);
    char imagebuffer[8]; // buffer to store 8 bytes of image
    int i = 0;
    while(i < enc_info->size_secret_file) // loops for the size of the secret file
    {
        fread(imagebuffer, 8, 1, enc_info->fptr_src_image); // reads 8 bytes from the source image
        if(encode_byte_to_lsb(enc_info->secret_data[i++], imagebuffer) == e_failure) // encodes the byte to lsb
            return e_failure;
        fwrite(imagebuffer, 8, 1, enc_info->fptr_dest_image); // writes the encoded data to the destination image
    }
    return e_success;
}

Status copy_remaining_img_data(EncodeInfo *enc_info) // copies the remaining data of the source image to the destination image
{
    puts(C "INFO:" Re " Coping Left Over Data");
#if 1
    int ch; // to store the byte read from the source image
    while((ch = fgetc(enc_info->fptr_src_image)) != EOF) // loops till the EOF reached
        fputc(ch, enc_info->fptr_dest_image); // writes the byte to the destination image
#endif
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer) // encodes a byte to the lsb of the image buffer
{
#if 1
    for(int i=7; i>=0; i--) // loops for 8 bits of the byte
        image_buffer[7-i] = (image_buffer[7-i] & ~1) | (data >> i & 1); // encodes the bit to lsb
    return e_success;
#endif
}

Status encode_size_to_lsb(int size, char *image_buffer) // encodes a size to the lsb of the image buffer
{
    for (int i=31; i>=0; i--) // loops for 32 bits of the size
        image_buffer[31-i] = (image_buffer[31-i] & ~1) | (size >> i & 1); // encodes the bit to lsb
    return e_success;
}

Status do_encoding(EncodeInfo *enc_info) // main function to do the encoding
{
    char ch;
    int i = 0;
    if(open_files(enc_info) == e_failure) // opens the files
        return e_failure;
    else
        puts(G "INFO:" Re " Done");

    puts(C "INFO:" Re " ## Encoding Procedure Started ##");
    if(check_capacity(enc_info) == e_failure) // checks the capacity of the source image to store the secret file
        return e_failure;
    else
        puts(G "INFO:" Re " Done. Found OK");

    if(copy_bmp_header(enc_info->fptr_src_image, enc_info->fptr_dest_image) == e_failure) // copies the header of the source image to the destination image
        return e_failure;
    else
        puts(G "INFO:" Re " Done");
#if 1
    if(encode_magic_string(MAGIC_STRING, enc_info) == e_failure) // encodes the magic string to the destination image
        return e_failure;
    else
        puts(G "INFO:" Re " Done");

    while((ch = fgetc(enc_info->fptr_secret)) != EOF) // loops till the EOF reached
        enc_info->secret_data[i++] = ch; // stores the byte to the secret data array
    enc_info->secret_data[i] = '\0'; // null terminate the string

    if(encode_secret_file_extn_size(strlen(enc_info->extn_secret_file), enc_info) == e_failure) // encodes the size of the extension of the secret file
        return e_failure;
    else
        puts(G "INFO:" Re " Done");
    if(encode_secret_file_extn(enc_info->extn_secret_file, enc_info) == e_failure) // encodes the extension of the secret file
        return e_failure;
    else
        puts(G "INFO:" Re " Done");

    if(encode_secret_file_size(enc_info->size_secret_file, enc_info) == e_failure) // encodes the size of the secret file
        return e_failure;
    else
        puts(G "INFO:" Re " Done");

    if(encode_secret_file_data(enc_info) == e_failure) // encodes the data of the secret file
        return e_failure;
    else
        puts(G "INFO:" Re " Done");
#endif

    if(copy_remaining_img_data(enc_info) == e_failure) // copies the remaining data of the source image to the destination image
        return e_failure;
    else
        puts(G "INFO:" Re " Done");

    return e_success;
}
