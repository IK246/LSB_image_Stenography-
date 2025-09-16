/*
Name : Karthick I
Date : 28/08/25
Title : Stegnography Project
*/

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

// function to check operation type
OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{

    if(argc >= 3) // checks for the number for arguments
    {
        if(check_operation_type(argv[1]) == e_encode) // calls the check operation functoin
        {
            EncodeInfo enc_info; // encode structure declaration
            if(read_and_validate_encode_args(argv, &enc_info) == e_success) // calls the validate argument function
            {
                if(do_encoding(&enc_info) == e_success) // calls the encoding function
                    puts(G "INFO:" Re " ## Encoding Done Successfully ##");
                else
                    puts(R "INFO:" Re " ## ERROR while Encoding Data ##");
            }
            else
                puts(R "INFO:" Re " ## ERROR while Encoding Data ##");
        }
        else if(check_operation_type(argv[1]) == e_decode) // calls the check operation function
        {
            DecodeInfo dec_info; //  decode structure declaration
            if(read_and_validate_decode_args(argv, &dec_info) == e_success) // calls the validate argument function
            {
                if(do_decoding(&dec_info) == e_success) // calls the decoding function
                    puts(G "INFO:" Re " ## Decoding Done Successfully ##");
                else
                    puts(R "INFO:" Re " ## ERROR while Decoding Data ##");
            }
            else
                puts(R "INFO:" Re " ## ERROR while Decoding Data ##");
        }
        else
            puts(R "INFO:" Re " Invalid mode");
    }
    else
        puts(R "INFO:" Re " ## ERROR insufficient Data ##");
    return 0;
}

OperationType check_operation_type(char *symbol)
{
    if(symbol[1] == 'e' && symbol[2] == '\0') // checks e in the argument
        return e_encode;
    else if(symbol[1] == 'd' && symbol[2] == '\0') // checks d in the argument
        return e_decode;
    else
        return e_unsupported;
}
