/*
 * Tests for the lib_xlsx_writer library.
 *
 * Copyright 2014, John McNamara, jmcnamara@cpan.org
 *
 */

#include "../ctest.h"
#include "../helper.h"

#include "xlsxwriter/workbook.h"

// Test the _write_workbook() function.
CTEST(workbook, write_workbook) {

    char* got;
    char exp[] = "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
    FILE* testfile = tmpfile();

    lxw_workbook *workbook = new_workbook(NULL);
    workbook->file = testfile;

    _write_workbook(workbook);

    RUN_XLSX_STREQ(exp, got);

    _free_workbook(workbook);
}

