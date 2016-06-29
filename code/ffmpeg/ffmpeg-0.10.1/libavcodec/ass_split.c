﻿/*
 * SSA/ASS spliting functions
 * Copyright (c) 2010  Aurelien Jacobs <aurel@gnuage.org>
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

#include "avcodec.h"
#include "ass_split.h"

typedef enum
{
    ASS_STR,
    ASS_INT,
    ASS_FLT,
    ASS_COLOR,
    ASS_TIMESTAMP,
    ASS_ALGN,
} ASSFieldType;

typedef struct
{
    const char *name;
    int type;
    int offset;
} ASSFields;

typedef struct
{
    const char *section;
    const char *format_header;
    const char *fields_header;
    int         size;
    int         offset;
    int         offset_count;
    ASSFields   fields[10];
} ASSSection;

static const ASSSection ass_sections[] =
{
    {
        .section       = "Script Info",
        .offset        = offsetof(ASS, script_info),
        .fields = {{"ScriptType", ASS_STR, offsetof(ASSScriptInfo, script_type)},
            {"Collisions", ASS_STR, offsetof(ASSScriptInfo, collisions) },
            {"PlayResX",   ASS_INT, offsetof(ASSScriptInfo, play_res_x) },
            {"PlayResY",   ASS_INT, offsetof(ASSScriptInfo, play_res_y) },
            {"Timer",      ASS_FLT, offsetof(ASSScriptInfo, timer)      },
            {0},
        }
    },
    {
        .section       = "V4+ Styles",
        .format_header = "Format",
        .fields_header = "Style",
        .size          = sizeof(ASSStyle),
        .offset        = offsetof(ASS, styles),
        .offset_count  = offsetof(ASS, styles_count),
        .fields = {{"Name",         ASS_STR,  offsetof(ASSStyle, name)         },
            {"Fontname",     ASS_STR,  offsetof(ASSStyle, font_name)    },
            {"Fontsize",     ASS_INT,  offsetof(ASSStyle, font_size)    },
            {"PrimaryColour",ASS_COLOR,offsetof(ASSStyle, primary_color)},
            {"BackColour",   ASS_COLOR,offsetof(ASSStyle, back_color)   },
            {"Bold",         ASS_INT,  offsetof(ASSStyle, bold)         },
            {"Italic",       ASS_INT,  offsetof(ASSStyle, italic)       },
            {"Underline",    ASS_INT,  offsetof(ASSStyle, underline)    },
            {"Alignment",    ASS_INT,  offsetof(ASSStyle, alignment)    },
            {0},
        }
    },
    {
        .section       = "V4 Styles",
        .format_header = "Format",
        .fields_header = "Style",
        .size          = sizeof(ASSStyle),
        .offset        = offsetof(ASS, styles),
        .offset_count  = offsetof(ASS, styles_count),
        .fields = {{"Name",         ASS_STR,  offsetof(ASSStyle, name)         },
            {"Fontname",     ASS_STR,  offsetof(ASSStyle, font_name)    },
            {"Fontsize",     ASS_INT,  offsetof(ASSStyle, font_size)    },
            {"PrimaryColour",ASS_COLOR,offsetof(ASSStyle, primary_color)},
            {"BackColour",   ASS_COLOR,offsetof(ASSStyle, back_color)   },
            {"Bold",         ASS_INT,  offsetof(ASSStyle, bold)         },
            {"Italic",       ASS_INT,  offsetof(ASSStyle, italic)       },
            {"Alignment",    ASS_ALGN, offsetof(ASSStyle, alignment)    },
            {0},
        }
    },
    {
        .section       = "Events",
        .format_header = "Format",
        .fields_header = "Dialogue",
        .size          = sizeof(ASSDialog),
        .offset        = offsetof(ASS, dialogs),
        .offset_count  = offsetof(ASS, dialogs_count),
        .fields = {{"Layer",  ASS_INT,        offsetof(ASSDialog, layer)       },
            {"Start",  ASS_TIMESTAMP,  offsetof(ASSDialog, start)       },
            {"End",    ASS_TIMESTAMP,  offsetof(ASSDialog, end)         },
            {"Style",  ASS_STR,        offsetof(ASSDialog, style)       },
            {"Text",   ASS_STR,        offsetof(ASSDialog, text)        },
            {0},
        }
    },
};


typedef int (*ASSConvertFunc)(void *dest, const char *buf, int len);

static int convert_str(void *dest, const char *buf, int len)
{
    char *str = av_malloc(len + 1);
    if (str)
    {
        memcpy(str, buf, len);
        str[len] = 0;
        if (*(void **)dest)
            av_free(*(void **)dest);
        *(char **)dest = str;
    }
    return !str;
}
static int convert_int(void *dest, const char *buf, int len)
{
    return sscanf(buf, "%d", (int *)dest) == 1;
}
static int convert_flt(void *dest, const char *buf, int len)
{
    return sscanf(buf, "%f", (float *)dest) == 1;
}
static int convert_color(void *dest, const char *buf, int len)
{
    return sscanf(buf, "&H%8x", (int *)dest) == 1 ||
           sscanf(buf, "%d",    (int *)dest) == 1;
}
static int convert_timestamp(void *dest, const char *buf, int len)
{
    int c, h, m, s, cs;
    if ((c = sscanf(buf, "%d:%02d:%02d.%02d", &h, &m, &s, &cs)) == 4)
        *(int *)dest = 360000*h + 6000*m + 100*s + cs;
    return c == 4;
}
static int convert_alignment(void *dest, const char *buf, int len)
{
    int a;
    if (sscanf(buf, "%d", &a) == 1)
    {
        /* convert V4 Style alignment to V4+ Style */
        *(int *)dest = a + ((a&4) >> 1) - 5*!!(a&8);
        return 1;
    }
    return 0;
}

static const ASSConvertFunc convert_func[] =
{
    [ASS_STR]       = convert_str,
    [ASS_INT]       = convert_int,
    [ASS_FLT]       = convert_flt,
    [ASS_COLOR]     = convert_color,
    [ASS_TIMESTAMP] = convert_timestamp,
    [ASS_ALGN]      = convert_alignment,
};


struct ASSSplitContext
{
    ASS ass;
    int current_section;
    int field_number[FF_ARRAY_ELEMS(ass_sections)];
    int *field_order[FF_ARRAY_ELEMS(ass_sections)];
};


static uint8_t *realloc_section_array(ASSSplitContext *ctx)
{
    const ASSSection *section = &ass_sections[ctx->current_section];
    int *count = (int *)((uint8_t *)&ctx->ass + section->offset_count);
    void **section_ptr = (void **)((uint8_t *)&ctx->ass + section->offset);
    uint8_t *tmp = av_realloc(*section_ptr, (*count+1)*section->size);
    if (!tmp)
        return NULL;
    *section_ptr = tmp;
    tmp += *count * section->size;
    memset(tmp, 0, section->size);
    (*count)++;
    return tmp;
}

static inline int is_eol(char buf)
{
    return buf == '\r' || buf == '\n' || buf == 0;
}

static inline const char *skip_space(const char *buf)
{
    while (*buf == ' ')
        buf++;
    return buf;
}

static const char *ass_split_section(ASSSplitContext *ctx, const char *buf)
{
    const ASSSection *section = &ass_sections[ctx->current_section];
    int *number = &ctx->field_number[ctx->current_section];
    int *order = ctx->field_order[ctx->current_section];
    int *tmp, i, len;

    while (buf && *buf)
    {
        if (buf[0] == '[')
        {
            ctx->current_section = -1;
            break;
        }
        if (buf[0] == ';' || (buf[0] == '!' && buf[1] == ':'))
        {
            /* skip comments */
        }
        else if (section->format_header && !order)
        {
            len = strlen(section->format_header);
            if (strncmp(buf, section->format_header, len) || buf[len] != ':')
                return NULL;
            buf += len + 1;
            while (!is_eol(*buf))
            {
                buf = skip_space(buf);
                len = strcspn(buf, ", \r\n");
                if (!(tmp = av_realloc(order, (*number + 1) * sizeof(*order))))
                    return NULL;
                order = tmp;
                order[*number] = -1;
                for (i=0; section->fields[i].name; i++)
                    if (!strncmp(buf, section->fields[i].name, len))
                    {
                        order[*number] = i;
                        break;
                    }
                (*number)++;
                buf = skip_space(buf + len + 1);
            }
            ctx->field_order[ctx->current_section] = order;
        }
        else if (section->fields_header)
        {
            len = strlen(section->fields_header);
            if (!strncmp(buf, section->fields_header, len) && buf[len] == ':')
            {
                uint8_t *ptr, *struct_ptr = realloc_section_array(ctx);
                if (!struct_ptr)  return NULL;
                buf += len + 1;
                for (i=0; !is_eol(*buf) && i < *number; i++)
                {
                    int last = i == *number - 1;
                    buf = skip_space(buf);
                    len = strcspn(buf, last ? "\r\n" : ",\r\n");
                    if (order[i] >= 0)
                    {
                        ASSFieldType type = section->fields[order[i]].type;
                        ptr = struct_ptr + section->fields[order[i]].offset;
                        convert_func[type](ptr, buf, len);
                    }
                    buf = skip_space(buf + len + !last);
                }
            }
        }
        else
        {
            len = strcspn(buf, ":\r\n");
            if (buf[len] == ':')
            {
                for (i=0; section->fields[i].name; i++)
                    if (!strncmp(buf, section->fields[i].name, len))
                    {
                        ASSFieldType type = section->fields[i].type;
                        uint8_t *ptr = (uint8_t *)&ctx->ass + section->offset;
                        ptr += section->fields[i].offset;
                        buf = skip_space(buf + len + 1);
                        convert_func[type](ptr, buf, strcspn(buf, "\r\n"));
                        break;
                    }
            }
        }
        buf += strcspn(buf, "\n") + 1;
    }
    return buf;
}

static int ass_split(ASSSplitContext *ctx, const char *buf)
{
    char c, section[16];
    int i;

    if (ctx->current_section >= 0)
        buf = ass_split_section(ctx, buf);

    while (buf && *buf)
    {
        if (sscanf(buf, "[%15[0-9A-Za-z+ ]]%c", section, &c) == 2)
        {
            buf += strcspn(buf, "\n") + 1;
            for (i=0; i<FF_ARRAY_ELEMS(ass_sections); i++)
                if (!strcmp(section, ass_sections[i].section))
                {
                    ctx->current_section = i;
                    buf = ass_split_section(ctx, buf);
                }
        }
        else
            buf += strcspn(buf, "\n") + 1;
    }
    return buf ? 0 : AVERROR_INVALIDDATA;
}

ASSSplitContext *ff_ass_split(const char *buf)
{
    ASSSplitContext *ctx = av_mallocz(sizeof(*ctx));
    ctx->current_section = -1;
    if (ass_split(ctx, buf) < 0)
    {
        ff_ass_split_free(ctx);
        return NULL;
    }
    return ctx;
}

static void free_section(ASSSplitContext *ctx, const ASSSection *section)
{
    uint8_t *ptr = (uint8_t *)&ctx->ass + section->offset;
    int i, j, *count, c = 1;

    if (section->format_header)
    {
        ptr   = *(void **)ptr;
        count = (int *)((uint8_t *)&ctx->ass + section->offset_count);
    }
    else
        count = &c;

    if (ptr)
        for (i=0; i<*count; i++, ptr += section->size)
            for (j=0; section->fields[j].name; j++)
            {
                const ASSFields *field = &section->fields[j];
                if (field->type == ASS_STR)
                    av_freep(ptr + field->offset);
            }
    *count = 0;

    if (section->format_header)
        av_freep((uint8_t *)&ctx->ass + section->offset);
}

ASSDialog *ff_ass_split_dialog(ASSSplitContext *ctx, const char *buf,
                               int cache, int *number)
{
    ASSDialog *dialog = NULL;
    int i, count;
    if (!cache)
        for (i=0; i<FF_ARRAY_ELEMS(ass_sections); i++)
            if (!strcmp(ass_sections[i].section, "Events"))
            {
                free_section(ctx, &ass_sections[i]);
                break;
            }
    count = ctx->ass.dialogs_count;
    if (ass_split(ctx, buf) == 0)
        dialog = ctx->ass.dialogs + count;
    if (number)
        *number = ctx->ass.dialogs_count - count;
    return dialog;
}

void ff_ass_split_free(ASSSplitContext *ctx)
{
    if (ctx)
    {
        int i;
        for (i=0; i<FF_ARRAY_ELEMS(ass_sections); i++)
            free_section(ctx, &ass_sections[i]);
        av_free(ctx);
    }
}


int ff_ass_split_override_codes(const ASSCodesCallbacks *callbacks, void *priv,
                                const char *buf)
{
    const char *text = NULL;
    char new_line[2];
    int text_len = 0;

    while (*buf)
    {
        if (text && callbacks->text &&
                (sscanf(buf, "\\%1[nN]", new_line) == 1 ||
                 !strncmp(buf, "{\\", 2)))
        {
            callbacks->text(priv, text, text_len);
            text = NULL;
        }
        if (sscanf(buf, "\\%1[nN]", new_line) == 1)
        {
            if (callbacks->new_line)
                callbacks->new_line(priv, new_line[0] == 'N');
            buf += 2;
        }
        else if (!strncmp(buf, "{\\", 2))
        {
            buf++;
            while (*buf == '\\')
            {
                char style[2], c[2], sep[2], c_num[2] = "0", tmp[128] = {0};
                unsigned int color = 0xFFFFFFFF;
                int len, size = -1, an = -1, alpha = -1;
                int x1, y1, x2, y2, t1 = -1, t2 = -1;
                if (sscanf(buf, "\\%1[bisu]%1[01\\}]%n", style, c, &len) > 1)
                {
                    int close = c[0] == '0' ? 1 : c[0] == '1' ? 0 : -1;
                    len += close != -1;
                    if (callbacks->style)
                        callbacks->style(priv, style[0], close);
                }
                else if (sscanf(buf, "\\c%1[\\}]%n", sep, &len) > 0 ||
                         sscanf(buf, "\\c&H%X&%1[\\}]%n", &color, sep, &len) > 1 ||
                         sscanf(buf, "\\%1[1234]c%1[\\}]%n", c_num, sep, &len) > 1 ||
                         sscanf(buf, "\\%1[1234]c&H%X&%1[\\}]%n", c_num, &color, sep, &len) > 2)
                {
                    if (callbacks->color)
                        callbacks->color(priv, color, c_num[0] - '0');
                }
                else if (sscanf(buf, "\\alpha%1[\\}]%n", sep, &len) > 0 ||
                         sscanf(buf, "\\alpha&H%2X&%1[\\}]%n", &alpha, sep, &len) > 1 ||
                         sscanf(buf, "\\%1[1234]a%1[\\}]%n", c_num, sep, &len) > 1 ||
                         sscanf(buf, "\\%1[1234]a&H%2X&%1[\\}]%n", c_num, &alpha, sep, &len) > 2)
                {
                    if (callbacks->alpha)
                        callbacks->alpha(priv, alpha, c_num[0] - '0');
                }
                else if (sscanf(buf, "\\fn%1[\\}]%n", sep, &len) > 0 ||
                         sscanf(buf, "\\fn%127[^\\}]%1[\\}]%n", tmp, sep, &len) > 1)
                {
                    if (callbacks->font_name)
                        callbacks->font_name(priv, tmp[0] ? tmp : NULL);
                }
                else if (sscanf(buf, "\\fs%1[\\}]%n", sep, &len) > 0 ||
                         sscanf(buf, "\\fs%u%1[\\}]%n", &size, sep, &len) > 1)
                {
                    if (callbacks->font_size)
                        callbacks->font_size(priv, size);
                }
                else if (sscanf(buf, "\\a%1[\\}]%n", sep, &len) > 0 ||
                         sscanf(buf, "\\a%2u%1[\\}]%n", &an, sep, &len) > 1 ||
                         sscanf(buf, "\\an%1[\\}]%n", sep, &len) > 0 ||
                         sscanf(buf, "\\an%1u%1[\\}]%n", &an, sep, &len) > 1)
                {
                    if (an != -1 && buf[2] != 'n')
                        an = (an&3) + (an&4 ? 6 : an&8 ? 3 : 0);
                    if (callbacks->alignment)
                        callbacks->alignment(priv, an);
                }
                else if (sscanf(buf, "\\r%1[\\}]%n", sep, &len) > 0 ||
                         sscanf(buf, "\\r%127[^\\}]%1[\\}]%n", tmp, sep, &len) > 1)
                {
                    if (callbacks->cancel_overrides)
                        callbacks->cancel_overrides(priv, tmp);
                }
                else if (sscanf(buf, "\\move(%d,%d,%d,%d)%1[\\}]%n", &x1, &y1, &x2, &y2, sep, &len) > 4 ||
                         sscanf(buf, "\\move(%d,%d,%d,%d,%d,%d)%1[\\}]%n", &x1, &y1, &x2, &y2, &t1, &t2, sep, &len) > 6)
                {
                    if (callbacks->move)
                        callbacks->move(priv, x1, y1, x2, y2, t1, t2);
                }
                else if (sscanf(buf, "\\pos(%d,%d)%1[\\}]%n", &x1, &y1, sep, &len) > 2)
                {
                    if (callbacks->move)
                        callbacks->move(priv, x1, y1, x1, y1, -1, -1);
                }
                else if (sscanf(buf, "\\org(%d,%d)%1[\\}]%n", &x1, &y1, sep, &len) > 2)
                {
                    if (callbacks->origin)
                        callbacks->origin(priv, x1, y1);
                }
                else
                {
                    len = strcspn(buf+1, "\\}") + 2;  /* skip unknown code */
                }
                buf += len - 1;
            }
            if (*buf++ != '}')
                return AVERROR_INVALIDDATA;
        }
        else
        {
            if (!text)
            {
                text = buf;
                text_len = 1;
            }
            else
                text_len++;
            buf++;
        }
    }
    if (text && callbacks->text)
        callbacks->text(priv, text, text_len);
    if (callbacks->end)
        callbacks->end(priv);
    return 0;
}

ASSStyle *ass_style_get(ASSSplitContext *ctx, const char *style)
{
    ASS *ass = &ctx->ass;
    int i;

    if (!style || !*style)
        style = "Default";
    for (i=0; i<ass->styles_count; i++)
        if (!strcmp(ass->styles[i].name, style))
            return ass->styles + i;
    return NULL;
}
