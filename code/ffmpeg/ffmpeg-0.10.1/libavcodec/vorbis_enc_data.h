﻿/*
 * copyright (c) 2006 Oded Shimon <ods15@ods15.dyndns.org>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVCODEC_VORBIS_ENC_DATA_H
#define AVCODEC_VORBIS_ENC_DATA_H

#include <stdint.h>

static const uint8_t codebook0[] =
{
    2, 10,  8, 14,  7, 12, 11, 14,  1,  5,  3,  7,  4,  9,  7, 13,
};

static const uint8_t codebook1[] =
{
    1,  4,  2,  6,  3,  7,  5,  7,
};

static const uint8_t codebook2[] =
{
    1,  5,  7, 21,  5,  8,  9, 21, 10,  9, 12, 20, 20, 16, 20,
    20,  4,  8,  9, 20,  6,  8,  9, 20, 11, 11, 13, 20, 20, 15,
    17, 20,  9, 11, 14, 20,  8, 10, 15, 20, 11, 13, 15, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 13, 20, 20, 20, 18, 18, 20, 20,
    20, 20, 20, 20,  3,  6,  8, 20,  6,  7,  9, 20, 10,  9, 12,
    20, 20, 20, 20, 20,  5,  7,  9, 20,  6,  6,  9, 20, 10,  9,
    12, 20, 20, 20, 20, 20,  8, 10, 13, 20,  8,  9, 12, 20, 11,
    10, 12, 20, 20, 20, 20, 20, 18, 20, 20, 20, 15, 17, 18, 20,
    18, 17, 18, 20, 20, 20, 20, 20,  7, 10, 12, 20,  8,  9, 11,
    20, 14, 13, 14, 20, 20, 20, 20, 20,  6,  9, 12, 20,  7,  8,
    11, 20, 12, 11, 13, 20, 20, 20, 20, 20,  9, 11, 15, 20,  8,
    10, 14, 20, 12, 11, 14, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 11, 16, 18,
    20, 15, 15, 17, 20, 20, 17, 20, 20, 20, 20, 20, 20,  9, 14,
    16, 20, 12, 12, 15, 20, 17, 15, 18, 20, 20, 20, 20, 20, 16,
    19, 18, 20, 15, 16, 20, 20, 17, 17, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20,
};

static const uint8_t codebook3[] =
{
    2,  3,  7, 13,  4,  4,  7, 15,  8,  6,  9, 17, 21, 16, 15,
    21,  2,  5,  7, 11,  5,  5,  7, 14,  9,  7, 10, 16, 17, 15,
    16, 21,  4,  7, 10, 17,  7,  7,  9, 15, 11,  9, 11, 16, 21,
    18, 15, 21, 18, 21, 21, 21, 15, 17, 17, 19, 21, 19, 18, 20,
    21, 21, 21, 20,
};

static const uint8_t codebook4[] =
{
    5,  5,  5,  5,  6,  5,  6,  5,  6,  5,  6,  5,  6,  5,  6,
    5,  6,  5,  6,  5,  6,  5,  6,  5,  7,  5,  7,  5,  7,  5,
    7,  5,  8,  6,  8,  6,  8,  6,  9,  6,  9,  6, 10,  6, 10,
    6, 11,  6, 11,  7, 11,  7, 12,  7, 12,  7, 12,  7, 12,  7,
    12,  7, 12,  7, 12,  7, 12,  8, 13,  8, 12,  8, 12,  8, 13,
    8, 13,  9, 13,  9, 13,  9, 13,  9, 12, 10, 12, 10, 13, 10,
    14, 11, 14, 12, 14, 13, 14, 13, 14, 14, 15, 16, 15, 15, 15,
    14, 15, 17, 21, 22, 22, 21, 22, 22, 22, 22, 22, 22, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21,
};

static const uint8_t codebook5[] =
{
    2,  5,  5,  4,  5,  4,  5,  4,  5,  4,  6,  5,  6,  5,  6,
    5,  6,  5,  7,  5,  7,  6,  8,  6,  8,  6,  8,  6,  9,  6,
    9,  6,
};

static const uint8_t codebook6[] =
{
    8,  5,  8,  4,  9,  4,  9,  4,  9,  4,  9,  4,  9,  4,  9,
    4,  9,  4,  9,  4,  9,  4,  8,  4,  8,  4,  9,  5,  9,  5,
    9,  5,  9,  5,  9,  6, 10,  6, 10,  7, 10,  8, 11,  9, 11,
    11, 12, 13, 12, 14, 13, 15, 13, 15, 14, 16, 14, 17, 15, 17,
    15, 15, 16, 16, 15, 16, 16, 16, 15, 18, 16, 15, 17, 17, 19,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19,
};

static const uint8_t codebook7[] =
{
    1,  5,  5,  5,  5,  5,  5,  5,  6,  5,  6,  5,  6,  5,  6,
    5,  6,  6,  7,  7,  7,  7,  8,  7,  8,  8,  9,  8, 10,  9,
    10,  9,
};

static const uint8_t codebook8[] =
{
    4,  3,  4,  3,  4,  4,  5,  4,  5,  4,  5,  5,  6,  5,  6,
    5,  7,  5,  7,  6,  7,  6,  8,  7,  8,  7,  8,  7,  9,  8,
    9,  9,  9,  9, 10, 10, 10, 11,  9, 12,  9, 12,  9, 15, 10,
    14,  9, 13, 10, 13, 10, 12, 10, 12, 10, 13, 10, 12, 11, 13,
    11, 14, 12, 13, 13, 14, 14, 13, 14, 15, 14, 16, 13, 13, 14,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 15, 15,
};

static const uint8_t codebook9[] =
{
    4,  5,  4,  5,  3,  5,  3,  5,  3,  5,  4,  4,  4,  4,  5,
    5,  5,
};

static const uint8_t codebook10[] =
{
    3,  3,  4,  3,  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  5,
    7,  5,  8,  6,  8,  6,  9,  7, 10,  7, 10,  8, 10,  8, 11,
    9, 11,
};

static const uint8_t codebook11[] =
{
    3,  7,  3,  8,  3, 10,  3,  8,  3,  9,  3,  8,  4,  9,  4,
    9,  5,  9,  6, 10,  6,  9,  7, 11,  7, 12,  9, 13, 10, 13,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12,
};

static const uint8_t codebook12[] =
{
    4,  5,  4,  5,  4,  5,  4,  5,  3,  5,  3,  5,  3,  5,  4,
    5,  4,
};

static const uint8_t codebook13[] =
{
    4,  2,  4,  2,  5,  3,  5,  4,  6,  6,  6,  7,  7,  8,  7,
    8,  7,  8,  7,  9,  8,  9,  8,  9,  8, 10,  8, 11,  9, 12,
    9, 12,
};

static const uint8_t codebook14[] =
{
    2,  5,  2,  6,  3,  6,  4,  7,  4,  7,  5,  9,  5, 11,  6,
    11,  6, 11,  7, 11,  6, 11,  6, 11,  9, 11,  8, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10,
    10, 10, 10,
};

static const uint8_t codebook15[] =
{
    5,  6, 11, 11, 11, 11, 10, 10, 12, 11,  5,  2, 11,  5,  6,
    6,  7,  9, 11, 13, 13, 10,  7, 11,  6,  7,  8,  9, 10, 12,
    11,  5, 11,  6,  8,  7,  9, 11, 14, 15, 11,  6,  6,  8,  4,
    5,  7,  8, 10, 13, 10,  5,  7,  7,  5,  5,  6,  8, 10, 11,
    10,  7,  7,  8,  6,  5,  5,  7,  9,  9, 11,  8,  8, 11,  8,
    7,  6,  6,  7,  9, 12, 11, 10, 13,  9,  9,  7,  7,  7,  9,
    11, 13, 12, 15, 12, 11,  9,  8,  8,  8,
};

static const uint8_t codebook16[] =
{
    2,  4,  4,  0,  0,  0,  0,  0,  0,  5,  6,  6,  0,  0,  0,
    0,  0,  0,  5,  6,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  5,  7,  7,  0,  0,  0,  0,  0,  0,
    7,  8,  8,  0,  0,  0,  0,  0,  0,  6,  7,  8,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  7,  7,
    0,  0,  0,  0,  0,  0,  6,  8,  7,  0,  0,  0,  0,  0,  0,
    7,  8,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  7,  7,  0,  0,  0,
    0,  0,  0,  7,  8,  8,  0,  0,  0,  0,  0,  0,  7,  8,  8,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    7,  8,  8,  0,  0,  0,  0,  0,  0,  8,  8,  9,  0,  0,  0,
    0,  0,  0,  8,  9,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  6,  8,  8,  0,  0,  0,  0,  0,  0,
    7,  9,  8,  0,  0,  0,  0,  0,  0,  8,  9,  9,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  5,  7,  7,  0,  0,  0,  0,  0,  0,  7,  8,  8,
    0,  0,  0,  0,  0,  0,  7,  8,  8,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  8,  8,  0,  0,  0,
    0,  0,  0,  8,  9,  9,  0,  0,  0,  0,  0,  0,  7,  8,  9,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    6,  8,  8,  0,  0,  0,  0,  0,  0,  8,  9,  9,  0,  0,  0,
    0,  0,  0,  8,  9,  8,
};

static const uint8_t codebook17[] =
{
    2,  5,  5,  0,  0,  0,  5,  5,  0,  0,  0,  5,  5,  0,  0,
    0,  7,  8,  0,  0,  0,  0,  0,  0,  0,  5,  6,  6,  0,  0,
    0,  7,  7,  0,  0,  0,  7,  7,  0,  0,  0, 10, 10,  0,  0,
    0,  0,  0,  0,  0,  5,  6,  6,  0,  0,  0,  7,  7,  0,  0,
    0,  7,  7,  0,  0,  0, 10, 10,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    5,  7,  7,  0,  0,  0,  7,  7,  0,  0,  0,  7,  7,  0,  0,
    0,  9,  9,  0,  0,  0,  0,  0,  0,  0,  5,  7,  7,  0,  0,
    0,  7,  7,  0,  0,  0,  7,  7,  0,  0,  0,  9,  9,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  5,  7,  7,  0,  0,  0,  7,  7,  0,  0,
    0,  7,  7,  0,  0,  0,  9,  9,  0,  0,  0,  0,  0,  0,  0,
    5,  7,  7,  0,  0,  0,  7,  7,  0,  0,  0,  7,  7,  0,  0,
    0,  9,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8, 10, 10,  0,  0,
    0,  9,  9,  0,  0,  0,  9,  9,  0,  0,  0, 10, 10,  0,  0,
    0,  0,  0,  0,  0,  8, 10, 10,  0,  0,  0,  9,  9,  0,  0,
    0,  9,  9,  0,  0,  0, 10, 10,
};

static const uint8_t codebook18[] =
{
    2,  4,  3,  6,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  4,  4,  6,  6,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  4,  4,  4,  6,  6,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    6,  6,  6,  9,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  6,  7,  9,  9,
};

static const uint8_t codebook19[] =
{
    2,  3,  3,  6,  6,  0,  0,  0,  0,  0,  4,  4,  6,  6,  0,
    0,  0,  0,  0,  4,  4,  6,  6,  0,  0,  0,  0,  0,  5,  5,
    6,  6,  0,  0,  0,  0,  0,  0,  0,  6,  6,  0,  0,  0,  0,
    0,  0,  0,  7,  8,  0,  0,  0,  0,  0,  0,  0,  7,  7,  0,
    0,  0,  0,  0,  0,  0,  9,  9,
};

static const uint8_t codebook20[] =
{
    1,  3,  4,  6,  6,  7,  7,  9,  9,  0,  5,  5,  7,  7,  7,
    8,  9,  9,  0,  5,  5,  7,  7,  8,  8,  9,  9,  0,  7,  7,
    8,  8,  8,  8, 10, 10,  0,  0,  0,  8,  8,  8,  8, 10, 10,
    0,  0,  0,  9,  9,  9,  9, 10, 10,  0,  0,  0,  9,  9,  9,
    9, 10, 10,  0,  0,  0, 10, 10, 10, 10, 11, 11,  0,  0,  0,
    0,  0, 10, 10, 11, 11,
};

static const uint8_t codebook21[] =
{
    2,  3,  3,  6,  6,  7,  7,  8,  8,  8,  8,  9,  9, 10, 10,
    11, 10,  0,  5,  5,  7,  7,  8,  8,  9,  9,  9,  9, 10, 10,
    10, 10, 11, 11,  0,  5,  5,  7,  7,  8,  8,  9,  9,  9,  9,
    10, 10, 10, 10, 11, 11,  0,  6,  6,  7,  7,  8,  8,  9,  9,
    9,  9, 10, 10, 11, 11, 11, 11,  0,  0,  0,  7,  7,  8,  8,
    9,  9,  9,  9, 10, 10, 11, 11, 11, 12,  0,  0,  0,  8,  8,
    8,  8,  9,  9,  9,  9, 10, 10, 11, 11, 12, 12,  0,  0,  0,
    8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 11, 11, 12, 12,  0,
    0,  0,  9,  9,  9,  9, 10, 10, 10, 10, 11, 10, 11, 11, 12,
    12,  0,  0,  0,  0,  0,  9,  9, 10, 10, 10, 10, 11, 11, 11,
    11, 12, 12,  0,  0,  0,  0,  0,  9,  8,  9,  9, 10, 10, 11,
    11, 12, 12, 12, 12,  0,  0,  0,  0,  0,  8,  8,  9,  9, 10,
    10, 11, 11, 12, 11, 12, 12,  0,  0,  0,  0,  0,  9, 10, 10,
    10, 11, 11, 11, 11, 12, 12, 13, 13,  0,  0,  0,  0,  0,  0,
    0, 10, 10, 10, 10, 11, 11, 12, 12, 13, 13,  0,  0,  0,  0,
    0,  0,  0, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13,  0,  0,
    0,  0,  0,  0,  0, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13,
    0,  0,  0,  0,  0,  0,  0, 11, 11, 12, 12, 12, 12, 13, 13,
    13, 13,  0,  0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12,
    13, 13, 13, 13,
};

static const uint8_t codebook22[] =
{
    1,  4,  4,  7,  6,  6,  7,  6,  6,  4,  7,  7, 10,  9,  9,
    11,  9,  9,  4,  7,  7, 10,  9,  9, 11,  9,  9,  7, 10, 10,
    11, 11, 10, 12, 11, 11,  6,  9,  9, 11, 10, 10, 11, 10, 10,
    6,  9,  9, 11, 10, 10, 11, 10, 10,  7, 11, 11, 11, 11, 11,
    12, 11, 11,  6,  9,  9, 11, 10, 10, 11, 10, 10,  6,  9,  9,
    11, 10, 10, 11, 10, 10,
};

static const uint8_t codebook23[] =
{
    2,  4,  4,  6,  6,  7,  7,  7,  7,  8,  8, 10,  5,  5,  6,
    6,  7,  7,  8,  8,  8,  8, 10,  5,  5,  6,  6,  7,  7,  8,
    8,  8,  8, 10,  6,  6,  7,  7,  8,  8,  8,  8,  8,  8, 10,
    10, 10,  7,  7,  8,  7,  8,  8,  8,  8, 10, 10, 10,  8,  8,
    8,  8,  8,  8,  8,  8, 10, 10, 10,  7,  8,  8,  8,  8,  8,
    8,  8, 10, 10, 10,  8,  8,  8,  8,  8,  8,  8,  8, 10, 10,
    10, 10, 10,  8,  8,  8,  8,  8,  8, 10, 10, 10, 10, 10,  9,
    9,  8,  8,  9,  8, 10, 10, 10, 10, 10,  8,  8,  8,  8,  8,
    8,
};

static const uint8_t codebook24[] =
{
    1,  4,  4,  6,  6,  7,  7,  8,  8,  9,  9, 10, 10,  6,  5,
    5,  7,  7,  8,  8,  8,  8,  9,  9, 10, 10,  7,  5,  5,  7,
    7,  8,  8,  8,  8,  9,  9, 11, 10,  0,  8,  8,  8,  8,  9,
    9,  9,  9, 10, 10, 11, 11,  0,  8,  8,  8,  8,  9,  9,  9,
    9, 10, 10, 11, 11,  0, 12, 12,  9,  9, 10, 10, 10, 10, 11,
    11, 11, 12,  0, 13, 13,  9,  9, 10, 10, 10, 10, 11, 11, 12,
    12,  0,  0,  0, 10, 10, 10, 10, 11, 11, 12, 12, 12, 12,  0,
    0,  0, 10, 10, 10, 10, 11, 11, 12, 12, 12, 12,  0,  0,  0,
    14, 14, 11, 11, 11, 11, 12, 12, 13, 13,  0,  0,  0, 14, 14,
    11, 11, 11, 11, 12, 12, 13, 13,  0,  0,  0,  0,  0, 12, 12,
    12, 12, 13, 13, 14, 13,  0,  0,  0,  0,  0, 13, 13, 12, 12,
    13, 12, 14, 13,
};

static const uint8_t codebook25[] =
{
    2,  4,  4,  5,  5,  6,  5,  5,  5,  5,  6,  4,  5,  5,  5,
    6,  5,  5,  5,  5,  6,  6,  6,  5,  5,
};

static const uint8_t codebook26[] =
{
    1,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,  4,  9,
    8, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,  2,  9,  7, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 11, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11,
};

static const uint8_t codebook27[] =
{
    1,  4,  4,  6,  6,  7,  7,  8,  7,  9,  9, 10, 10, 10, 10,
    6,  5,  5,  7,  7,  8,  8, 10,  8, 11, 10, 12, 12, 13, 13,
    6,  5,  5,  7,  7,  8,  8, 10,  9, 11, 11, 12, 12, 13, 12,
    18,  8,  8,  8,  8,  9,  9, 10,  9, 11, 10, 12, 12, 13, 13,
    18,  8,  8,  8,  8,  9,  9, 10, 10, 11, 11, 13, 12, 14, 13,
    18, 11, 11,  9,  9, 10, 10, 11, 11, 11, 12, 13, 12, 13, 14,
    18, 11, 11,  9,  8, 11, 10, 11, 11, 11, 11, 12, 12, 14, 13,
    18, 18, 18, 10, 11, 10, 11, 12, 12, 12, 12, 13, 12, 14, 13,
    18, 18, 18, 10, 11, 11,  9, 12, 11, 12, 12, 12, 13, 13, 13,
    18, 18, 17, 14, 14, 11, 11, 12, 12, 13, 12, 14, 12, 14, 13,
    18, 18, 18, 14, 14, 11, 10, 12,  9, 12, 13, 13, 13, 13, 13,
    18, 18, 17, 16, 18, 13, 13, 12, 12, 13, 11, 14, 12, 14, 14,
    17, 18, 18, 17, 18, 13, 12, 13, 10, 12, 11, 14, 14, 14, 14,
    17, 18, 18, 18, 18, 15, 16, 12, 12, 13, 10, 14, 12, 14, 15,
    18, 18, 18, 16, 17, 16, 14, 12, 11, 13, 10, 13, 13, 14, 15,
};

static const uint8_t codebook28[] =
{
    2,  5,  5,  6,  6,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,
    8,  8, 10,  6,  6,  7,  7,  8,  7,  8,  8,  8,  8,  8,  9,
    9,  9,  9,  9, 10,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,
    9,  9,  9,  9,  9,  9, 10,  7,  7,  7,  7,  8,  8,  8,  8,
    9,  9,  9,  9,  9,  9,  9,  9, 10, 10, 10,  7,  7,  8,  8,
    8,  9,  9,  9,  9,  9,  9,  9,  9,  9, 11, 11, 11,  8,  8,
    8,  8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 10, 10, 10,
    8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 10,
    10, 10,  8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 10,
    9, 10, 10, 10, 11, 11,  9,  9,  9,  9,  9,  9,  9,  9,  9,
    9,  9,  9, 11, 10, 11, 11, 11,  9,  9,  9,  9,  9,  9, 10,
    10,  9,  9, 10,  9, 11, 10, 11, 11, 11,  9,  9,  9,  9,  9,
    9,  9,  9, 10, 10, 10,  9, 11, 11, 11, 11, 11,  9,  9,  9,
    9, 10, 10,  9,  9,  9,  9, 10,  9, 11, 11, 11, 11, 11, 11,
    11,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 11, 11, 11, 11,
    11, 11, 11, 10,  9, 10, 10,  9, 10,  9,  9, 10,  9, 11, 10,
    10, 11, 11, 11, 11,  9, 10,  9,  9,  9,  9, 10, 10, 10, 10,
    11, 11, 11, 11, 11, 11, 10, 10, 10,  9,  9, 10,  9, 10,  9,
    10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11,  9,  9,  9,  9,
    9, 10, 10, 10,
};

static const struct
{
    int dim;
    int len;
    int real_len;
    const uint8_t *clens;
    int lookup;
    float min;
    float delta;
    const uint8_t *quant;
} cvectors[] =
{
    { 2,   16,   16, codebook0,  0 },
    { 2,    8,    8, codebook1,  0 },
    { 2,  256,  256, codebook2,  0 },
    { 2,   64,   64, codebook3,  0 },
    { 2,  128,  128, codebook4,  0 },
    { 2,   32,   32, codebook5,  0 },
    { 2,   96,   96, codebook6,  0 },
    { 2,   32,   32, codebook7,  0 },
    { 2,   96,   96, codebook8,  0 },
    { 2,   17,   17, codebook9,  0 },
    { 2,   32,   32, codebook10, 0 },
    { 2,   78,   78, codebook11, 0 },
    { 2,   17,   17, codebook12, 0 },
    { 2,   32,   32, codebook13, 0 },
    { 2,   78,   78, codebook14, 0 },
    { 2,  100,  100, codebook15, 0 },
    {
        8, 1641, 6561, codebook16, 1,    -1.0,   1.0, (const uint8_t[])
        {
            1, 0, 2,
        }
    },
    {
        4,  443,  625, codebook17, 1,    -2.0,   1.0, (const uint8_t[])
        {
            2, 1, 3, 0, 4,
        }
    },
    {
        4,  105,  625, codebook18, 1,    -2.0,   1.0, (const uint8_t[])
        {
            2, 1, 3, 0, 4,
        }
    },
    {
        2,   68,   81, codebook19, 1,    -4.0,   1.0, (const uint8_t[])
        {
            4, 3, 5, 2, 6, 1, 7, 0, 8,
        }
    },
    {
        2,   81,   81, codebook20, 1,    -4.0,   1.0, (const uint8_t[])
        {
            4, 3, 5, 2, 6, 1, 7, 0, 8,
        }
    },
    {
        2,  289,  289, codebook21, 1,    -8.0,   1.0, (const uint8_t[])
        {
            8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15, 0, 16,
        }
    },
    {
        4,   81,   81, codebook22, 1,   -11.0,  11.0, (const uint8_t[])
        {
            1, 0, 2,
        }
    },
    {
        2,  121,  121, codebook23, 1,    -5.0,   1.0, (const uint8_t[])
        {
            5, 4, 6, 3, 7, 2, 8, 1, 9, 0, 10,
        }
    },
    {
        2,  169,  169, codebook24, 1,   -30.0,   5.0, (const uint8_t[])
        {
            6, 5, 7, 4, 8, 3, 9, 2, 10, 1, 11, 0, 12,
        }
    },
    {
        2,   25,   25, codebook25, 1,    -2.0,   1.0, (const uint8_t[])
        {
            2, 1, 3, 0, 4,
        }
    },
    {
        2,  169,  169, codebook26, 1, -1530.0, 255.0, (const uint8_t[])
        {
            6, 5, 7, 4, 8, 3, 9, 2, 10, 1, 11, 0, 12,
        }
    },
    {
        2,  225,  225, codebook27, 1,  -119.0,  17.0, (const uint8_t[])
        {
            7, 6, 8, 5, 9, 4, 10, 3, 11, 2, 12, 1, 13, 0, 14,
        }
    },
    {
        2,  289,  289, codebook28, 1,    -8.0,   1.0, (const uint8_t[])
        {
            8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15, 0, 16,
        }
    },
};

static const struct
{
    int dim;
    int subclass;
    int masterbook;
    const int nbooks[4];
} floor_classes[] =
{
    { 3, 0, 0, {  4             } },
    { 4, 1, 0, {  5,  6         } },
    { 3, 1, 1, {  7,  8         } },
    { 4, 2, 2, { -1,  9, 10, 11 } },
    { 3, 2, 3, { -1, 12, 13, 14 } },
};

#endif /* AVCODEC_VORBIS_ENC_DATA_H */
