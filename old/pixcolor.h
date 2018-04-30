// -*- mode: c++ -*-
/** https://pixlark.github.io/
 *
 ** pixcolor.h
 * 
 * RGB/HSV conversions. Code from https://stackoverflow.com/a/14733008
 * and written by Leszek S.
 *
 */

#ifndef PIXLIB_PIXCOLOR_H
#define PIXLIB_PIXCOLOR_H

#include "pixint.h"

typedef struct RGBColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGBColor;

typedef struct HSVColor
{
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HSVColor;

RGBColor HSVtoRGB(HSVColor hsv);

HSVColor RGBtoHSV(RGBColor rgb);

#endif
