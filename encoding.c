#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define UTF8BOM 0xEFBBBF
#define UTF16LE 0xFFFE
#define UTF16BE 0xFEFF
#define UTF32BE 0x0000FEFF
#define UTF32LE 0xFFFE0000

void terminate_error_reading() {
    printf("ERROR: Could not read a file\n");
    exit(EXIT_FAILURE);
}
void terminate_error_writing() {
    printf("ERROR: Could not write to a file\n");
    exit(EXIT_FAILURE);
}
void terminate_error_opening() {
    printf("ERROR: Could not open file\n");
    exit(EXIT_FAILURE);
}
void terminate_error_allocationg_memory() {
    printf("ERROR: Could not allocate memory\n");
    exit(EXIT_FAILURE);
}


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
    if (file == NULL) terminate_error_opening();
    unsigned short dword;
    if (!fread(&dword, 1, 2, file)) terminate_error_reading();
    dword = endian_switch16(dword);
    switch(dword) {
        case 0xFFFE: {
            if (!fread(&dword, 1, 2, file)) terminate_error_reading();
            fclose(file);
            dword = endian_switch16(dword);
            if (dword == 0) {
                return 4;
            } else {
                return 2;
            }
        }
        case 0xFEFF: {
            fclose(file);
            return 3;
        }
        case 0x0000: {
            if (!fread(&dword, 1, 2, file)) terminate_error_reading();
            fclose(file);
            dword = endian_switch16(dword);
            if (dword == 0xFEFF) {
                return 5;
            } else {
                return 0;
            }
        }
        case 0xEFBB: {
            dword = 0;
            if (!fread(&dword, 1, 1, file)) terminate_error_reading();
            fclose(file);
            if (dword == 0xBF) {
                return 1;
            } else {
                return 0;
            }
        }
        default: {
            fclose(file);
            return 0;
        }
    }
}

unsigned int get_symbol8(FILE * file) {
    unsigned int result = 0, byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
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
    unsigned int result = 0;
    unsigned short word1 = 0, word2 = 0;
    if (!fread(&word1, 1, 2, file)) terminate_error_reading();
    if (word1 < 0xD800 || word1 > 0xDFFF) {
        result = word1;
        return result;
    } else if (word1 >= 0xDC00) {
        return result;
    } else {
        result = (word1 & 0x3ff) << 10;
        if (!fread(&word2, 1, 2, file)) terminate_error_reading();
        if (word2 < 0xDC00 || word2 > 0xDFFF) {
            return 0;
        }
        result |= (word2 & 0x3FF);
        result += 0x10000;
        return result;
    }
}
unsigned int get_symbol16BE(FILE * file) {
    unsigned int result = 0;
    unsigned short word1 = 0, word2 = 0;
    if (!fread(&word1, 1, 2, file)) terminate_error_reading();
    word1 = endian_switch16(word1);
    if (word1 < 0xD800 || word1 > 0xDFFF) {
        result = word1;
        return result;
    } else if (word1 < 0xDC00) {
        return 0;
    } else {
        if (!fread(&word2, 1, 2, file)) terminate_error_reading();
        word2 = endian_switch16(word2);
        if (word2 >= 0xDC00 && word2 <= 0xDFFF) {
            return 0;
        }
        result = (word2 & 0x3ff) << 10;
        result |= (word1 & 0x3FF);
        result += 0x10000;
        return result;
    }
}
unsigned int get_symbol32LE(FILE * file) {
    unsigned int code = 0;
    if (!fread(&code, 4, 1, file)) terminate_error_reading();
    return code;
}
unsigned int get_symbol32BE(FILE * file) {
    unsigned int code = 0;
    if (!fread(&code, 4, 1, file)) terminate_error_reading();
    code = endian_switch32(code);
    return code;
}

void put_BOM(FILE * file, int type) {
    unsigned int BOM = 0;
    switch(type) {
        case 1: {
            BOM = endian_switch32(UTF8BOM);
            BOM >>= 8;
            if (!fwrite(&BOM, 1, 3, file)) terminate_error_writing();
            break;
        }
        case 2: {
            BOM = endian_switch32(UTF16LE);
            BOM >>= 16;
            if (!fwrite(&BOM, 1, 2, file)) terminate_error_writing();
            break;
        }
        case 3: {
            BOM = endian_switch32(UTF16BE);
            BOM >>= 16;
            if (!fwrite(&BOM, 1, 2, file)) terminate_error_writing();
            break;
        }
        case 4: {
            BOM = endian_switch32(UTF32LE);
            if (!fwrite(&BOM, 1, 4, file)) terminate_error_writing();
            break;
        }
        case 5: {
            BOM = endian_switch32(UTF32BE);
            if(!fwrite(&BOM, 1, 4, file)) terminate_error_writing();
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
        if (!fwrite(&dword1, 2, 1, file)) terminate_error_writing();
        if (!fwrite(&dword2, 2, 1, file)) terminate_error_writing();
    }
}

void put_symbol16BE(FILE * file, unsigned int code) {
    if (code < 0x10000) {
        unsigned short dword = endian_switch16(code);
        if(!fwrite(&dword, 2, 1, file)) terminate_error_writing();
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
    fwrite(&code, 4, 1, file);
}

void put_symbol32BE(FILE * file, unsigned int code) {
    code = endian_switch32(code);
    if (!fwrite(&code, 4, 1, file)) terminate_error_writing();
}

void put_symbol8(FILE * file, unsigned int code) {
    unsigned int seq = 0;
    if (code < 0x7F) {
        if (!fwrite(&code, 1, 1, file)) terminate_error_writing();
        return;
    } else if (code < 0x7FF) {
        seq = 0xC080;
        seq |= (code % 0x40);
        code >>= 6;
        seq |= (code << 8);
        seq = endian_switch32(seq);
        seq >>= 16;
        if(!fwrite(&seq, 1, 2, file)) terminate_error_writing();
        return;
    } else if (code < 0xFFFF) {
        seq = 0xE08080;
        seq |= (code % 0x40);
        code >>= 6;
        seq |= (code % 0x40) << 8;
        code >>= 6;
        seq |= code << 16;
        seq = endian_switch32(seq);
        seq >>= 8;
        if(!fwrite(&seq, 1, 3, file)) terminate_error_writing();
        return;
    } else if (code < 0x10FFFF) {
        seq = 0xF0808080;
        seq |= (code % 0x40);
        code >>= 6;
        seq |= (code % 0x40) << 8;
        code >>= 6;
        seq |= (code % 0x40) << 16;
        code >>= 6;
        seq |= code << 24;
        seq = endian_switch32(seq);
        if(!fwrite(&seq, 1, 4, file)) terminate_error_writing();
        return;
    } else {
        return;
    }
}


