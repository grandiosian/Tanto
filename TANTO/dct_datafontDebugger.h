#ifndef DCT_DATAFONTDEBUGMODE_H
#define DCT_DATAFONTDEBUGMODE_H


typedef struct {
    unsigned short charcode;  // Utiliser unsigned short pour gérer les codes > 127
    int offset;
} CharMapEntry;

extern const unsigned char FONT_COMPLETE[];

extern const CharMapEntry CHAR_MAP_COMPLETE[];

extern const int TOTAL_CHARS;

#endif