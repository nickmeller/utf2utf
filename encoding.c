#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define UTF8BOM 0x00EFBBBF
#define UTF16LE 0x0000FFFE
#define UTF16BE 0x0000FEFF
#define UTF32LE 0x0000FEFF
#define UTF32BE 0xFFFE0000

// TODO: create error functions


unsigned short endian_switch16(unsigned short num) {
    return (num >> 8) | (num << 8);
}

unsigned int endian_switch32(unsigned int num) {
    return ((num >> 24) & 0xff) | // move byte 3 to byte 0
           ((num << 8) & 0xff0000) | // move byte 1 to byte 2
           ((num >> 8) & 0xff00) | // move byte 2 to byte 1
           ((num << 24) & 0xff000000);
}

int get_UTF_type(char * file_in) {
    FILE * file = fopen(file_in, "r");
    if (file == NULL) {
        printf("ERROR: Could not open the input file");
        return 1;
    }
    unsigned int BOM;
    fread(&BOM, sizeof(int), 1, file);
    fclose(file);
    BOM = endian_switch32(BOM);
    switch(BOM) {
        case UTF32LE: return 4;
        case UTF32BE: return 5;
        default: BOM >>= 8;
    }
    if (BOM == UTF8BOM)
        return 1;
    BOM >>= 8;
    switch (BOM) {
        case UTF16LE: return 2;
        case UTF16BE: return 3;
        default: return 0;
    }
}

char * file_copy(char * file_in) {
    FILE * file = fopen(file_in, "rb");
    if (file == NULL) {
        printf("ERROR: could not open file\n");
        return NULL;
    }
    FILE * copy = fopen("__cpy", "w");
    if (copy == NULL) {
        printf("ERROR: Unable to create temperate files\n");
        return NULL;
    }
    short ch;
    while((ch = fgetc(file)) != EOF)
    {
        fputc(ch, copy);
    }
    fclose(file);
    fclose(copy);
    return "__cpy";
}


unsigned int get_symbol8(FILE * file) {
    unsigned int result, byte1, byte2, byte3, byte4;
    byte1 = fgetc(file);
    if ((byte1 >> 7) == 0b0) {
        result = byte1;
    } else if ((byte1 >> 5) == 0b110) {
        byte1 <<= 27;
        byte1 >>= 27;
        byte2 = fgetc(file);
        byte2 <<= 26;
        byte2 >>= 26;
        result = (byte1 << 6) + byte2;
    } else if ((byte1 >> 4) == 0b1110) {
        byte1 <<= 28;
        byte1 >>= 28;
        byte2 = fgetc(file);
        byte2 <<= 26;
        byte2 >>= 26;
        byte3 = fgetc(file);
        byte3 <<= 26;
        byte3 >>= 26;
        result = (byte1 << 12) + (byte2 << 6) + (byte1);
    } else if ((byte1 >> 3) == 0b11110) {
        byte1 <<= 29;
        byte1 >>= 29;
        byte2 = fgetc(file);
        byte2 <<= 26;
        byte2 >>= 26;
        byte3 = fgetc(file);
        byte3 <<= 26;
        byte3 >>= 26;
        byte4 = fgetc(file);
        byte4 <<= 26;
        byte4 >>= 26;
        result = (byte1 << 18) + (byte2 << 12) + (byte3 << 6) + byte4;
    } else {
        byte1 <<= 25;
        byte1 >>= 25;
        result = 0xDC80 + byte1;
    }
    return result;
}

unsigned int get_symbol16LE(FILE * file) {
    // TODO: write function, reading 16LE
    return 0;
}
unsigned int get_symbol16BE(FILE * file) {
    // TODO: write function
    return 0;
}
unsigned int get_symbol32LE(FILE * file) {
    return 0;
    // TODO: write function
}
unsigned int get_symbol32BE(FILE * file) {
    return 0;
    // TODO: write function
}

void put_BOM(FILE * file, int type) {
    unsigned int BOM;
    switch(type) {
        case 1: {
            BOM = endian_switch32(UTF8BOM);
            BOM >>= 8;
            fwrite(&BOM, 1, 3, file);
            break;
        }
        case 2: {
            BOM = endian_switch32(UTF16LE);
            BOM >>= 16;
            fwrite(&BOM, 1, 2, file);
            break;
        }
        case 3: {
            BOM = endian_switch32(UTF16BE);
            BOM >>= 16;
            fwrite(&BOM, 1, 2, file);
            break;
        }
        case 4: {
            BOM = endian_switch32(UTF32LE);
            fwrite(&BOM, 1, 4, file);
            break;
        }
        case 5: {
            BOM = endian_switch32(UTF16BE);
            fwrite(&BOM, 1, 4, file);
            break;
        }
        default: {
            break;
        }
    }
}

void put_symbol16LE(FILE * file, unsigned int code) {
    if (code < 0x10000) {
        unsigned short dword = (unsigned short)code;
        fwrite(&dword, 2, 1, file);
    } else {
        code-= 0x10000;
        unsigned char low_10 = (unsigned char) (code & 0x3FF);
        unsigned char high_10 = (unsigned char) (code >> 10);
        unsigned short dword1 = (unsigned short) (0xD800 | high_10);
        unsigned short dword2 = (unsigned short) (0xDC00 | low_10);
        fwrite(&dword1, 2, 1, file);
        fwrite(&dword2, 2, 1, file);
    }
}

void put_symbol16BE(FILE * file, unsigned int code) {
    if (code < 0x10000) {
        unsigned short dword = endian_switch16(code);
        fwrite(&dword, 2, 1, file);
    } else {
        code-= 0x10000;
        unsigned char low_10 = (unsigned char) (code & 0x3FF);
        unsigned char high_10 = (unsigned char) (code >> 10);
        unsigned short dword1 = endian_switch16(0xD800 | high_10);
        unsigned short dword2 = endian_switch16(0xDC00 | low_10);
        fwrite(&dword2, 2, 1, file);
        fwrite(&dword1, 2, 1, file);
    }
}
void put_symbol32LE(FILE * file, unsigned int code) {
    // TODO: write function
}
void put_symbol32BE(FILE * file, unsigned int code) {
    // TODO: write function
}
void put_symbol8(FILE * file, unsigned int code) {
    // TODO: write function
}


