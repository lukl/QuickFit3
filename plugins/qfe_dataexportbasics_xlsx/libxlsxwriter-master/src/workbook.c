/*****************************************************************************
 * workbook - A library for creating Excel XLSX workbook files.
 *
 * Used in conjunction with the libxlsxwriter library.
 *
 * Copyright 2014-2015, John McNamara, jmcnamara@cpan.org. See LICENSE.txt.
 *
 */

#include "xlsxwriter/xmlwriter.h"
#include "xlsxwriter/workbook.h"
#include "xlsxwriter/utility.h"
#include "xlsxwriter/packager.h"
#include "xlsxwriter/hash_table.h"

/*
 * Forward declarations.
 */

/*****************************************************************************
 *
 * Private functions.
 *
 ****************************************************************************/

/*
 * Free a workbook object.
 */
void
_free_workbook(lxw_workbook *workbook)
{
    lxw_worksheet *worksheet;
    lxw_format *format;
    lxw_defined_name *defined_name;

    if (!workbook)
        return;

    if (workbook->properties) {
        free(workbook->properties->title);
        free(workbook->properties->subject);
        free(workbook->properties->author);
        free(workbook->properties->manager);
        free(workbook->properties->company);
        free(workbook->properties->category);
        free(workbook->properties->keywords);
        free(workbook->properties->comments);
        free(workbook->properties->status);
    }

    free(workbook->properties);

    /* Free the worksheets in the workbook. */
    while (!STAILQ_EMPTY(workbook->worksheets)) {
        worksheet = STAILQ_FIRST(workbook->worksheets);
        STAILQ_REMOVE_HEAD(workbook->worksheets, list_pointers);
        _free_worksheet(worksheet);
    }

    /* Free the formats in the workbook. */
    while (!STAILQ_EMPTY(workbook->formats)) {
        format = STAILQ_FIRST(workbook->formats);
        STAILQ_REMOVE_HEAD(workbook->formats, list_pointers);
        _free_format(format);
    }

    /* Free the defined_names in the workbook. */
    while (!TAILQ_EMPTY(workbook->defined_names)) {
        defined_name = TAILQ_FIRST(workbook->defined_names);
        TAILQ_REMOVE(workbook->defined_names, defined_name, list_pointers);
        free(defined_name);
    }

    _free_lxw_hash(workbook->used_xf_formats);
    _free_sst(workbook->sst);
    free(workbook->worksheets);
    free(workbook->formats);
    free(workbook->defined_names);
    free(workbook);
}

/*
 * Set the default index for each format. This is only used for testing.
 */
void
_set_default_xf_indices(lxw_workbook *self)
{
    lxw_format *format;

    STAILQ_FOREACH(format, self->formats, list_pointers) {
        _get_xf_index(format);
    }
}

/*
 * Iterate through the XF Format objects and give them an index to non-default
 * font elements.
 */
STATIC void
_prepare_fonts(lxw_workbook *self)
{

    lxw_hash_table *fonts = _new_lxw_hash(128, 1, 1);
    lxw_hash_element *hash_element;
    lxw_hash_element *used_format_element;
    uint16_t index = 0;

    LXW_FOREACH_ORDERED(used_format_element, self->used_xf_formats) {
        lxw_format *format = (lxw_format *) used_format_element->value;
        lxw_font *key = _get_font_key(format);

        if (key) {
            /* Look up the format in the hash table. */
            hash_element = _hash_key_exists(fonts, key, sizeof(lxw_font));

            if (hash_element) {
                /* Font has already been used. */
                format->font_index = *(uint16_t *) hash_element->value;
                format->has_font = LXW_FALSE;
                free(key);
            }
            else {
                /* This is a new font. */
                uint16_t *font_index = calloc(1, sizeof(uint16_t));
                *font_index = index;
                format->font_index = index;
                format->has_font = 1;
                _insert_hash_element(fonts, key, font_index,
                                     sizeof(lxw_font));
                index++;
            }
        }
    }

    _free_lxw_hash(fonts);

    self->font_count = index;
}

/*
 * Iterate through the XF Format objects and give them an index to non-default
 * border elements.
 */
STATIC void
_prepare_borders(lxw_workbook *self)
{

    lxw_hash_table *borders = _new_lxw_hash(128, 1, 1);
    lxw_hash_element *hash_element;
    lxw_hash_element *used_format_element;
    uint16_t index = 0;

    LXW_FOREACH_ORDERED(used_format_element, self->used_xf_formats) {
        lxw_format *format = (lxw_format *) used_format_element->value;
        lxw_border *key = _get_border_key(format);

        if (key) {
            /* Look up the format in the hash table. */
            hash_element = _hash_key_exists(borders, key, sizeof(lxw_border));

            if (hash_element) {
                /* Border has already been used. */
                format->border_index = *(uint16_t *) hash_element->value;
                format->has_border = LXW_FALSE;
                free(key);
            }
            else {
                /* This is a new border. */
                uint16_t *border_index = calloc(1, sizeof(uint16_t));
                *border_index = index;
                format->border_index = index;
                format->has_border = 1;
                _insert_hash_element(borders, key, border_index,
                                     sizeof(lxw_border));
                index++;
            }
        }
    }

    _free_lxw_hash(borders);

    self->border_count = index;
}

/*
 * Iterate through the XF Format objects and give them an index to non-default
 * fill elements.
 */
STATIC void
_prepare_fills(lxw_workbook *self)
{

    lxw_hash_table *fills = _new_lxw_hash(128, 1, 1);
    lxw_hash_element *hash_element;
    lxw_hash_element *used_format_element;
    uint16_t index = 2;
    lxw_fill *default_fill_1 = NULL;
    lxw_fill *default_fill_2 = NULL;
    uint16_t *fill_index1 = NULL;
    uint16_t *fill_index2 = NULL;

    default_fill_1 = calloc(1, sizeof(lxw_fill));
    GOTO_LABEL_ON_MEM_ERROR(default_fill_1, mem_error);

    default_fill_2 = calloc(1, sizeof(lxw_fill));
    GOTO_LABEL_ON_MEM_ERROR(default_fill_2, mem_error);

    fill_index1 = calloc(1, sizeof(uint16_t));
    GOTO_LABEL_ON_MEM_ERROR(fill_index1, mem_error);

    fill_index2 = calloc(1, sizeof(uint16_t));
    GOTO_LABEL_ON_MEM_ERROR(fill_index2, mem_error);

    /* Add the default fills. */
    default_fill_1->pattern = LXW_PATTERN_NONE;
    default_fill_1->fg_color = LXW_COLOR_UNSET;
    default_fill_1->bg_color = LXW_COLOR_UNSET;
    *fill_index1 = 0;
    _insert_hash_element(fills, default_fill_1, fill_index1,
                         sizeof(lxw_fill));

    default_fill_2->pattern = LXW_PATTERN_GRAY_125;
    default_fill_2->fg_color = LXW_COLOR_UNSET;
    default_fill_2->bg_color = LXW_COLOR_UNSET;
    *fill_index2 = 1;
    _insert_hash_element(fills, default_fill_2, fill_index2,
                         sizeof(lxw_fill));

    LXW_FOREACH_ORDERED(used_format_element, self->used_xf_formats) {
        lxw_format *format = (lxw_format *) used_format_element->value;
        lxw_fill *key = _get_fill_key(format);

        /* The following logical statements jointly take care of special */
        /* cases in relation to cell colours and patterns:               */
        /* 1. For a solid fill (pattern == 1) Excel reverses the role of */
        /*    foreground and background colours, and                     */
        /* 2. If the user specifies a foreground or background colour    */
        /*    withouth a pattern they probably wanted a solid fill, so   */
        /*    we fill in the defaults.                                   */
        if (format->pattern == LXW_PATTERN_SOLID
            && format->bg_color != LXW_COLOR_UNSET
            && format->fg_color != LXW_COLOR_UNSET) {
            lxw_color_t tmp = format->fg_color;
            format->fg_color = format->bg_color;
            format->bg_color = tmp;
        }

        if (format->pattern <= LXW_PATTERN_SOLID
            && format->bg_color != LXW_COLOR_UNSET
            && format->fg_color == LXW_COLOR_UNSET) {
            format->fg_color = format->bg_color;
            format->bg_color = LXW_COLOR_UNSET;
            format->pattern = LXW_PATTERN_SOLID;
        }

        if (format->pattern <= LXW_PATTERN_SOLID
            && format->bg_color == LXW_COLOR_UNSET
            && format->fg_color != LXW_COLOR_UNSET) {
            format->bg_color = LXW_COLOR_UNSET;
            format->pattern = LXW_PATTERN_SOLID;
        }

        if (key) {
            /* Look up the format in the hash table. */
            hash_element = _hash_key_exists(fills, key, sizeof(lxw_fill));

            if (hash_element) {
                /* Fill has already been used. */
                format->fill_index = *(uint16_t *) hash_element->value;
                format->has_fill = LXW_FALSE;
                free(key);
            }
            else {
                /* This is a new fill. */
                uint16_t *fill_index = calloc(1, sizeof(uint16_t));
                *fill_index = index;
                format->fill_index = index;
                format->has_fill = 1;
                _insert_hash_element(fills, key, fill_index,
                                     sizeof(lxw_fill));
                index++;
            }
        }
    }

    _free_lxw_hash(fills);

    self->fill_count = index;

    return;

mem_error:
    free(fill_index2);
    free(fill_index1);
    free(default_fill_2);
    free(default_fill_1);
    _free_lxw_hash(fills);
}

/*
 * Iterate through the XF Format objects and give them an index to non-default
 * number format elements. Note, user defined records start from index 0xA4.
 */
STATIC void
_prepare_num_formats(lxw_workbook *self)
{

    lxw_hash_table *num_formats = _new_lxw_hash(128, 0, 1);
    lxw_hash_element *hash_element;
    lxw_hash_element *used_format_element;
    uint16_t index = 0xA4;
    uint16_t num_format_count = 0;
    char *num_format;
    uint16_t *num_format_index;

    LXW_FOREACH_ORDERED(used_format_element, self->used_xf_formats) {
        lxw_format *format = (lxw_format *) used_format_element->value;

        /* Format already has a number format index. */
        if (format->num_format_index)
            continue;

        /* Check if there is a user defined number format string. */
        num_format = format->num_format;

        if (strlen(num_format)) {
            /* Look up the num_format in the hash table. */
            hash_element = _hash_key_exists(num_formats, num_format,
                                            strlen(num_format));

            if (hash_element) {
                /* Num_Format has already been used. */
                format->num_format_index = *(uint16_t *) hash_element->value;
            }
            else {
                /* This is a new num_format. */
                num_format_index = calloc(1, sizeof(uint16_t));
                *num_format_index = index;
                format->num_format_index = index;
                _insert_hash_element(num_formats, num_format,
                                     num_format_index, strlen(num_format));
                index++;
                num_format_count++;
            }
        }
    }

    _free_lxw_hash(num_formats);

    self->num_format_count = num_format_count;
}

/*
 * Prepare workbook and sub-objects for writing.
 */
STATIC void
_prepare_workbook(lxw_workbook *self)
{
    /* Set the font index for the format objects. */
    _prepare_fonts(self);

    /* Set the number format index for the format objects. */
    _prepare_num_formats(self);

    /* Set the border index for the format objects. */
    _prepare_borders(self);

    /* Set the fill index for the format objects. */
    _prepare_fills(self);

}

/*
 * Compare two defined_name structures.
 */
static int
_compare_defined_names(lxw_defined_name *a, lxw_defined_name *b)
{
    int res = strcmp(a->normalised_name, b->normalised_name);

    /* Primary comparison based on defined name. */
    if (res)
        return res;

    /* Secondary comparison based on worksheet name. */
    res = strcmp(a->normalised_sheetname, b->normalised_sheetname);

    return res;
}

/*
 * Process and store the defined names. The defined names are stored with
 * the Workbook.xml but also with the App.xml if they refer to a sheet
 * range like "Sheet1!:A1". The defined names are store in sorted
 * order for consistency with Excel. The names need to be normalised before
 * sorting.
 */
STATIC uint8_t
_store_defined_name(lxw_workbook *self, const char *name,
                    const char *app_name, const char *formula, int16_t index,
                    uint8_t hidden)
{
    lxw_worksheet *worksheet;
    lxw_defined_name *defined_name;
    lxw_defined_name *list_defined_name;
    char name_copy[LXW_DEFINED_NAME_LENGTH];
    char *tmp_str;
    char *worksheet_name;

    /* Do some checks on the input data */
    if (!name || !formula)
        return 1;

    if (strlen(name) > LXW_DEFINED_NAME_LENGTH ||
        strlen(formula) > LXW_DEFINED_NAME_LENGTH) {
        return 1;
    }

    /* Allocate a new defined_name to be added to the linked list of names. */
    defined_name = calloc(1, sizeof(struct lxw_defined_name));
    RETURN_ON_MEM_ERROR(defined_name, 1);

    /* Copy the user input string. */
    strcpy(name_copy, name);

    /* Set the worksheet index or -1 for a global defined name. */
    defined_name->index = index;
    defined_name->hidden = hidden;

    /* Check for local defined names like like "Sheet1!name". */
    tmp_str = strchr(name_copy, '!');

    if (tmp_str == NULL) {
        /* The name is global. We just store the defined name string. */
        strcpy(defined_name->name, name_copy);
    }
    else {
        /* The name is worksheet local. We need to extract the sheet name
         * and map it to a sheet index. */

        /* Split the into the worksheet name and defined name. */
        *tmp_str = '\0';
        tmp_str++;
        worksheet_name = name_copy;

        /* Remove any worksheet quoting. */
        if (worksheet_name[0] == '\'')
            worksheet_name++;
        if (worksheet_name[strlen(worksheet_name) - 1] == '\'')
            worksheet_name[strlen(worksheet_name) - 1] = '\0';

        /* Search for worksheet name to get the equivalent worksheet index. */
        STAILQ_FOREACH(worksheet, self->worksheets, list_pointers) {
            if (strcmp(worksheet_name, worksheet->name) == 0) {
                defined_name->index = worksheet->index;
                strcpy(defined_name->normalised_sheetname, worksheet_name);
            }
        }

        /* If we didn't find the worksheet name we exit. */
        if (defined_name->index == -1)
            goto mem_error;

        strcpy(defined_name->name, tmp_str);
    }

    /* Print titles and repeat title pass in the name used for App.xml. */
    if (app_name) {
        strcpy(defined_name->app_name, app_name);
        strcpy(defined_name->normalised_sheetname, app_name);
    }
    else {
        strcpy(defined_name->app_name, name);
    }

    /* We need to normalise the defined names for sorting. This involves
     * removing any _xlnm namespace  and converting it to lowercase. */
    tmp_str = strstr(name_copy, "_xlnm.");

    if (tmp_str)
        strcpy(defined_name->normalised_name, defined_name->name + 6);
    else
        strcpy(defined_name->normalised_name, defined_name->name);

    lxw_str_tolower(defined_name->normalised_name);
    lxw_str_tolower(defined_name->normalised_sheetname);

    /* Strip leading "=" from the formula. */
    if (formula[0] == '=')
        strcpy(defined_name->formula, formula + 1);
    else
        strcpy(defined_name->formula, formula);

    /* We add the defined name to the list in sorted order. */
    list_defined_name = TAILQ_FIRST(self->defined_names);

    if (list_defined_name == NULL ||
        _compare_defined_names(defined_name, list_defined_name) < 1) {
        /* List is empty or defined name goes to the head. */
        TAILQ_INSERT_HEAD(self->defined_names, defined_name, list_pointers);
        return 0;
    }

    TAILQ_FOREACH(list_defined_name, self->defined_names, list_pointers) {
        int res = _compare_defined_names(defined_name, list_defined_name);

        /* The entry already exists. We exit and don't overwrite. */
        if (res == 0)
            goto mem_error;

        /* New defined name is inserted in sorted order before other entries. */
        if (res < 0) {
            TAILQ_INSERT_BEFORE(list_defined_name, defined_name,
                                list_pointers);
            return 0;
        }
    }

    /* If the entry wasn't less than any of the entries in the list we add it
     * to the end. */
    TAILQ_INSERT_TAIL(self->defined_names, defined_name, list_pointers);
    return 0;

mem_error:
    free(defined_name);
    return 1;
}

/*
 * Iterate through the worksheets and store any defined names used for print
 * ranges or repeat rows/columns.
 */
STATIC void
_prepare_defined_names(lxw_workbook *self)
{
    lxw_worksheet *worksheet;
    char app_name[LXW_DEFINED_NAME_LENGTH];
    char range[LXW_DEFINED_NAME_LENGTH];
    char area[MAX_CELL_RANGE_LENGTH];
    char first_col[8];
    char last_col[8];

    STAILQ_FOREACH(worksheet, self->worksheets, list_pointers) {

        /*
         * Check for autofilter settings and store them.
         */
        if (worksheet->autofilter.in_use) {

            __builtin_snprintf(app_name, LXW_DEFINED_NAME_LENGTH - 1,
                               "%s!_FilterDatabase", worksheet->quoted_name);

            lxw_range_abs(area,
                          worksheet->autofilter.first_row,
                          worksheet->autofilter.first_col,
                          worksheet->autofilter.last_row,
                          worksheet->autofilter.last_col);

            __builtin_snprintf(range, LXW_DEFINED_NAME_LENGTH - 1, "%s!%s",
                               worksheet->quoted_name, area);

            /* Autofilters are the only defined name to set the hidden flag. */
            _store_defined_name(self, "_xlnm._FilterDatabase", app_name,
                                range, worksheet->index, LXW_TRUE);
        }

        /*
         * Check for Print Area settings and store them.
         */
        if (worksheet->print_area.in_use) {

            __builtin_snprintf(app_name, LXW_DEFINED_NAME_LENGTH - 1,
                               "%s!Print_Area", worksheet->quoted_name);

            /* Check for print area that is the max row range. */
            if (worksheet->print_area.first_row == 0
                && worksheet->print_area.last_row == LXW_ROW_MAX - 1) {

                lxw_col_to_name(first_col,
                                worksheet->print_area.first_col, LXW_FALSE);

                lxw_col_to_name(last_col,
                                worksheet->print_area.last_col, LXW_FALSE);

                __builtin_snprintf(area, MAX_CELL_RANGE_LENGTH - 1, "$%s:$%s",
                                   first_col, last_col);

            }
            /* Check for print area that is the max column range. */
            else if (worksheet->print_area.first_col == 0
                     && worksheet->print_area.last_col == LXW_COL_MAX - 1) {

                __builtin_snprintf(area, MAX_CELL_RANGE_LENGTH - 1, "$%d:$%d",
                                   worksheet->print_area.first_row + 1,
                                   worksheet->print_area.last_row + 1);

            }
            else {
                lxw_range_abs(area,
                              worksheet->print_area.first_row,
                              worksheet->print_area.first_col,
                              worksheet->print_area.last_row,
                              worksheet->print_area.last_col);
            }

            __builtin_snprintf(range, LXW_DEFINED_NAME_LENGTH - 1, "%s!%s",
                               worksheet->quoted_name, area);

            _store_defined_name(self, "_xlnm.Print_Area", app_name,
                                range, worksheet->index, LXW_FALSE);
        }

        /*
         * Check for repeat rows/cols. aka, Print Titles and store them.
         */
        if (worksheet->repeat_rows.in_use || worksheet->repeat_cols.in_use) {
            if (worksheet->repeat_rows.in_use
                && worksheet->repeat_cols.in_use) {
                __builtin_snprintf(app_name, LXW_DEFINED_NAME_LENGTH - 1,
                                   "%s!Print_Titles", worksheet->quoted_name);

                lxw_col_to_name(first_col,
                                worksheet->repeat_cols.first_col, LXW_FALSE);

                lxw_col_to_name(last_col,
                                worksheet->repeat_cols.last_col, LXW_FALSE);

                __builtin_snprintf(range, LXW_DEFINED_NAME_LENGTH - 1,
                                   "%s!$%s:$%s,%s!$%d:$%d",
                                   worksheet->quoted_name, first_col,
                                   last_col, worksheet->quoted_name,
                                   worksheet->repeat_rows.first_row + 1,
                                   worksheet->repeat_rows.last_row + 1);

                _store_defined_name(self, "_xlnm.Print_Titles", app_name,
                                    range, worksheet->index, LXW_FALSE);
            }
            else if (worksheet->repeat_rows.in_use) {

                __builtin_snprintf(app_name, LXW_DEFINED_NAME_LENGTH - 1,
                                   "%s!Print_Titles", worksheet->quoted_name);

                __builtin_snprintf(range, LXW_DEFINED_NAME_LENGTH - 1,
                                   "%s!$%d:$%d", worksheet->quoted_name,
                                   worksheet->repeat_rows.first_row + 1,
                                   worksheet->repeat_rows.last_row + 1);

                _store_defined_name(self, "_xlnm.Print_Titles", app_name,
                                    range, worksheet->index, LXW_FALSE);
            }
            else if (worksheet->repeat_cols.in_use) {
                __builtin_snprintf(app_name, LXW_DEFINED_NAME_LENGTH - 1,
                                   "%s!Print_Titles", worksheet->quoted_name);

                lxw_col_to_name(first_col,
                                worksheet->repeat_cols.first_col, LXW_FALSE);

                lxw_col_to_name(last_col,
                                worksheet->repeat_cols.last_col, LXW_FALSE);

                __builtin_snprintf(range, LXW_DEFINED_NAME_LENGTH - 1,
                                   "%s!$%s:$%s", worksheet->quoted_name,
                                   first_col, last_col);

                _store_defined_name(self, "_xlnm.Print_Titles", app_name,
                                    range, worksheet->index, LXW_FALSE);
            }
        }
    }
}

/*****************************************************************************
 *
 * XML functions.
 *
 ****************************************************************************/

/*
 * Write the XML declaration.
 */
STATIC void
_workbook_xml_declaration(lxw_workbook *self)
{
    _xml_declaration(self->file);
}

/*
 * Write the <workbook> element.
 */
STATIC void
_write_workbook(lxw_workbook *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char xmlns[] = "http://schemas.openxmlformats.org"
        "/spreadsheetml/2006/main";
    char xmlns_r[] = "http://schemas.openxmlformats.org"
        "/officeDocument/2006/relationships";

    _INIT_ATTRIBUTES();
    _PUSH_ATTRIBUTES_STR("xmlns", xmlns);
    _PUSH_ATTRIBUTES_STR("xmlns:r", xmlns_r);

    _xml_start_tag(self->file, "workbook", &attributes);

    _FREE_ATTRIBUTES();
}

/*
 * Write the <fileVersion> element.
 */
STATIC void
_write_file_version(lxw_workbook *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    _INIT_ATTRIBUTES();
    _PUSH_ATTRIBUTES_STR("appName", "xl");
    _PUSH_ATTRIBUTES_STR("lastEdited", "4");
    _PUSH_ATTRIBUTES_STR("lowestEdited", "4");
    _PUSH_ATTRIBUTES_STR("rupBuild", "4505");

    _xml_empty_tag(self->file, "fileVersion", &attributes);

    _FREE_ATTRIBUTES();
}

/*
 * Write the <workbookPr> element.
 */
STATIC void
_write_workbook_pr(lxw_workbook *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    _INIT_ATTRIBUTES();
    _PUSH_ATTRIBUTES_STR("defaultThemeVersion", "124226");

    _xml_empty_tag(self->file, "workbookPr", &attributes);

    _FREE_ATTRIBUTES();
}

/*
 * Write the <workbookView> element.
 */
STATIC void
_write_workbook_view(lxw_workbook *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    _INIT_ATTRIBUTES();
    _PUSH_ATTRIBUTES_STR("xWindow", "240");
    _PUSH_ATTRIBUTES_STR("yWindow", "15");
    _PUSH_ATTRIBUTES_STR("windowWidth", "16095");
    _PUSH_ATTRIBUTES_STR("windowHeight", "9660");

    if (self->first_sheet)
        _PUSH_ATTRIBUTES_INT("firstSheet", self->first_sheet);

    if (self->active_sheet)
        _PUSH_ATTRIBUTES_INT("activeTab", self->active_sheet);

    _xml_empty_tag(self->file, "workbookView", &attributes);

    _FREE_ATTRIBUTES();
}

/*
 * Write the <bookViews> element.
 */
STATIC void
_write_book_views(lxw_workbook *self)
{
    _xml_start_tag(self->file, "bookViews", NULL);

    _write_workbook_view(self);

    _xml_end_tag(self->file, "bookViews");
}

/*
 * Write the <sheet> element.
 */
STATIC void
_write_sheet(lxw_workbook *self, const char *name, uint32_t sheet_id,
             uint8_t hidden)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char r_id[MAX_ATTRIBUTE_LENGTH] = "rId1";

    __builtin_snprintf(r_id, ATTR_32, "rId%d", sheet_id);

    _INIT_ATTRIBUTES();
    _PUSH_ATTRIBUTES_STR("name", name);
    _PUSH_ATTRIBUTES_INT("sheetId", sheet_id);

    if (hidden)
        _PUSH_ATTRIBUTES_STR("state", "hidden");

    _PUSH_ATTRIBUTES_STR("r:id", r_id);

    _xml_empty_tag(self->file, "sheet", &attributes);

    _FREE_ATTRIBUTES();
}

/*
 * Write the <sheets> element.
 */
STATIC void
_write_sheets(lxw_workbook *self)
{
    lxw_worksheet *worksheet;

    _xml_start_tag(self->file, "sheets", NULL);

    STAILQ_FOREACH(worksheet, self->worksheets, list_pointers) {
        _write_sheet(self, worksheet->name, worksheet->index + 1,
                     worksheet->hidden);
    }

    _xml_end_tag(self->file, "sheets");
}

/*
 * Write the <calcPr> element.
 */
STATIC void
_write_calc_pr(lxw_workbook *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    _INIT_ATTRIBUTES();
    _PUSH_ATTRIBUTES_STR("calcId", "124519");
    _PUSH_ATTRIBUTES_STR("fullCalcOnLoad", "1");

    _xml_empty_tag(self->file, "calcPr", &attributes);

    _FREE_ATTRIBUTES();
}

/*
 * Write the <definedName> element.
 */
STATIC void
_write_defined_name(lxw_workbook *self, lxw_defined_name *defined_name)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    _INIT_ATTRIBUTES();
    _PUSH_ATTRIBUTES_STR("name", defined_name->name);

    if (defined_name->index != -1)
        _PUSH_ATTRIBUTES_INT("localSheetId", defined_name->index);

    if (defined_name->hidden)
        _PUSH_ATTRIBUTES_INT("hidden", 1);

    _xml_data_element(self->file, "definedName", defined_name->formula,
                      &attributes);

    _FREE_ATTRIBUTES();
}

/*
 * Write the <definedNames> element.
 */
STATIC void
_write_defined_names(lxw_workbook *self)
{
    lxw_defined_name *defined_name;

    if (TAILQ_EMPTY(self->defined_names))
        return;

    _xml_start_tag(self->file, "definedNames", NULL);

    TAILQ_FOREACH(defined_name, self->defined_names, list_pointers) {
        _write_defined_name(self, defined_name);
    }

    _xml_end_tag(self->file, "definedNames");
}

/*****************************************************************************
 *
 * XML file assembly functions.
 *
 ****************************************************************************/

/*
 * Assemble and write the XML file.
 */
void
_workbook_assemble_xml_file(lxw_workbook *self)
{
    /* Prepare workbook and sub-objects for writing. */
    _prepare_workbook(self);

    /* Write the XML declaration. */
    _workbook_xml_declaration(self);

    /* Write the root workbook element. */
    _write_workbook(self);

    /* Write the XLSX file version. */
    _write_file_version(self);

    /* Write the workbook properties. */
    _write_workbook_pr(self);

    /* Write the workbook view properties. */
    _write_book_views(self);

    /* Write the worksheet names and ids. */
    _write_sheets(self);

    /* Write the workbook defined names. */
    _write_defined_names(self);

    /* Write the workbook calculation properties. */
    _write_calc_pr(self);

    /* Close the workbook tag. */
    _xml_end_tag(self->file, "workbook");
}

/*****************************************************************************
 *
 * Public functions.
 *
 ****************************************************************************/

/*
 * Create a new workbook object.
 */
lxw_workbook *
new_workbook(const char *filename)
{
    return new_workbook_opt(filename, NULL);
}

/*
 * Create a new workbook object with options.
 */
lxw_workbook *
new_workbook_opt(const char *filename, lxw_workbook_options *options)
{
    lxw_format *format;
    lxw_workbook *workbook;

    /* Create the workbook object. */
    workbook = calloc(1, sizeof(lxw_workbook));
    GOTO_LABEL_ON_MEM_ERROR(workbook, mem_error);
    workbook->filename = filename;
    printf("\nwb-fn: %s\n", workbook->filename);

    /* Add the worksheets list. */
    workbook->worksheets = calloc(1, sizeof(struct lxw_worksheets));
    GOTO_LABEL_ON_MEM_ERROR(workbook->worksheets, mem_error);
    STAILQ_INIT(workbook->worksheets);

    /* Add the formats list. */
    workbook->formats = calloc(1, sizeof(struct lxw_formats));
    GOTO_LABEL_ON_MEM_ERROR(workbook->formats, mem_error);
    STAILQ_INIT(workbook->formats);

    /* Add the defined_names list. */
    workbook->defined_names = calloc(1, sizeof(struct lxw_defined_names));
    GOTO_LABEL_ON_MEM_ERROR(workbook->defined_names, mem_error);
    TAILQ_INIT(workbook->defined_names);

    /* Add the shared strings table. */
    workbook->sst = _new_sst();
    GOTO_LABEL_ON_MEM_ERROR(workbook->sst, mem_error);

    /* Add the default workbook properties. */
    workbook->properties = calloc(1, sizeof(lxw_doc_properties));
    GOTO_LABEL_ON_MEM_ERROR(workbook->properties, mem_error);

    /* Add a hash table to track format indices. */
    workbook->used_xf_formats = _new_lxw_hash(128, 1, 0);
    GOTO_LABEL_ON_MEM_ERROR(workbook->used_xf_formats, mem_error);

    /* Add the default cell format. */
    format = workbook_add_format(workbook);
    GOTO_LABEL_ON_MEM_ERROR(format, mem_error);

    /* Initialise its index. */
    _get_xf_index(format);

    if (options)
        workbook->options.constant_memory = options->constant_memory;

    return workbook;

mem_error:
    _free_workbook(workbook);
    workbook = NULL;
    return NULL;
}

/*
 * Add a new worksheet to the Excel workbook.
 */
lxw_worksheet *
workbook_add_worksheet(lxw_workbook *self, const char *sheetname)
{
    lxw_worksheet *worksheet;
    lxw_worksheet_init_data init_data;
    char *new_name = NULL;

    if (sheetname) {
        /* Use the user supplied name. */
        if (strlen(sheetname) > LXW_SHEETNAME_MAX) {
            return NULL;
        }
        else {
            init_data.name = lxw_strdup(sheetname);
            init_data.quoted_name = lxw_quote_sheetname((char *) sheetname);
        }
    }
    else {
        /* Use the default SheetN name. */
        new_name = malloc(LXW_SHEETNAME_LEN);
        RETURN_ON_MEM_ERROR(new_name, NULL);
        __builtin_snprintf(new_name, LXW_SHEETNAME_LEN, "Sheet%d",
                           self->num_sheets + 1);
        init_data.name = new_name;
        init_data.quoted_name = lxw_strdup(new_name);
    }

    init_data.hidden = 0;
    init_data.index = self->num_sheets;
    init_data.sst = self->sst;
    init_data.optimize = self->options.constant_memory;
    init_data.active_sheet = &self->active_sheet;

    /* Create a new worksheet object. */
    worksheet = _new_worksheet(&init_data);

    if (worksheet) {
        self->num_sheets++;
        STAILQ_INSERT_TAIL(self->worksheets, worksheet, list_pointers);
    }
    else {
        free(new_name);
    }

    return worksheet;
}

/*
 * Add a new format to the Excel workbook.
 */
lxw_format *
workbook_add_format(lxw_workbook *self)
{
    /* Create a new format object. */
    lxw_format *format = _new_format();
    RETURN_ON_MEM_ERROR(format, NULL);

    format->xf_format_indices = self->used_xf_formats;
    format->num_xf_formats = &self->num_xf_formats;

    STAILQ_INSERT_TAIL(self->formats, format, list_pointers);

    return format;
}

/*
 * Call finalisation code and close file.
 */
uint8_t
workbook_close(lxw_workbook *self)
{
    lxw_worksheet *worksheet;
    lxw_packager *packager;
    uint8_t error = 0;

    /* Add a default worksheet if non have been added. */
    if (!self->num_sheets)
        workbook_add_worksheet(self, NULL);

    /* Ensure that at least one worksheet has been selected. */
    if (self->active_sheet == 0) {
        worksheet = STAILQ_FIRST(self->worksheets);
        worksheet->selected = 1;
        worksheet->hidden = 0;
    }

    /* Set the active sheet. */
    STAILQ_FOREACH(worksheet, self->worksheets, list_pointers) {
        if (worksheet->index == self->active_sheet)
            worksheet->active = 1;
    }

    /* Set the defined names for the worksheets such as Print Titles. */
    _prepare_defined_names(self);

    packager = _new_packager(self->filename);
    GOTO_LABEL_ON_MEM_ERROR(packager, mem_error);

    packager->workbook = self;

    error = _create_package(packager);

    _free_packager(packager);
    _free_workbook(self);

    return error;

mem_error:
    return 1;
}

/*
 * Create a defined name in Excel. We handle global/workbook level names and
 * local/worksheet names.
 */
uint8_t
workbook_define_name(lxw_workbook *self, const char *name,
                     const char *formula)
{
    return _store_defined_name(self, name, NULL, formula, -1, LXW_FALSE);
}
