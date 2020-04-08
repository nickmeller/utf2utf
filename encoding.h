#ifndef UTF2UTF_ENCODING_H
#define UTF2UTF_ENCODING_H

void terminate_error_reading();
void terminate_error_writing();
void terminate_error_opening();
void terminate_error_allocationg_memory();

unsigned short endian_switch16(unsigned short num);
unsigned int endian_switch32(unsigned int num);
int get_UTF_type(char * file_in);
char * file_copy(char * file_in);

unsigned int get_symbol8(FILE * file);
unsigned int get_symbol16LE(FILE * file);
unsigned int get_symbol16BE(FILE * file);
unsigned int get_symbol32LE(FILE * file);
unsigned int get_symbol32BE(FILE * file);

void put_BOM(FILE * file, int type);

void put_symbol16LE(FILE * file, unsigned int code);
void put_symbol16BE(FILE * file, unsigned int code);
void put_symbol32LE(FILE * file, unsigned int code);
void put_symbol32BE(FILE * file, unsigned int code);
void put_symbol8(FILE * file, unsigned int code);




#endif //UTF2UTF_ENCODING_H
