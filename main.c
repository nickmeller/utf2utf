#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encoding.h"

int main(int argc, char ** argv) {
    if (argc != 4) {
        printf("ERROR: program should get exactly 3 arguments");
        return 1;
    }
    char * file_in = malloc(sizeof(char) * (strlen(argv[1]) + 1));
    strcpy(file_in, argv[1]);
    char * file_out = malloc(sizeof(char) * (strlen(argv[2]) + 1));
    strcpy(file_out, argv[2]);
    int resulting_encoding = atoi(argv[3]);
    int current_encoding = get_UTF_type(file_in);

    FILE * in = fopen(file_in, "r");
    if (in == NULL) {
        printf("ERROR: Could not open file %s", file_in);
        exit(EXIT_FAILURE);
    }
    FILE * out = fopen(file_out, "w");
    if (out == NULL) {
        printf("ERROR: Could not open file %s", file_out);
        exit(EXIT_FAILURE);
    }
    fseek(in, 0, SEEK_END);
    size_t file_size = ftell(in);
    long offset = 0;
    switch(current_encoding) {
        case 1: {
            offset = 3;
            break;
        }
        case 2:
        case 3: {
            offset = 2;
            break;
        }
        case 4:
        case 5: {
            offset = 4;
            break;
        }
        default: {
            break;
        }
    }
    printf("%ld\n", offset);
    fseek(in, offset, SEEK_SET);
    put_BOM(out, resulting_encoding);
    while (ftell(in) < file_size) {
        unsigned int code = 0;
        switch(current_encoding) {
            case 0:
            case 1: {
                code = get_symbol8(in);
                break;
            }
            case 2: {
                code = get_symbol16LE(in);
                break;
            }
            case 3: {
                code = get_symbol16BE(in);
                break;
            }
            case 4: {
                code = get_symbol32LE(in);
                break;
            }
            case 5: {
                code = get_symbol32BE(in);
                break;
            }
            default: {
                break;
            }
        }

        switch(resulting_encoding) {
            case 0:
            case 1: {
                put_symbol8(out, code);
                break;
            }
            case 2: {
                put_symbol16LE(out, code);
                break;
            }
            case 3: {
                put_symbol16BE(out, code);
                break;
            }
            case 4: {
                put_symbol32LE(out, code);
                break;
            }
            case 5: {
               put_symbol32BE(out, code);
               break;
            }
            default: {
                break;
            }
        }
    }

    fclose(in);
    fclose(out);

    free(file_in);
    free(file_out);

    exit(EXIT_SUCCESS);
}