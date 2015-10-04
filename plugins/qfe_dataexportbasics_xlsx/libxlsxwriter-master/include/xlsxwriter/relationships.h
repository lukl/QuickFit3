/*
 * libxlsxwriter
 * 
 * Copyright 2014-2015, John McNamara, jmcnamara@cpan.org. See LICENSE.txt.
 *
 * relationships - A libxlsxwriter library for creating Excel XLSX
 *                 relationships files.
 *
 */
#ifndef __LXW_RELATIONSHIPS_H__
#define __LXW_RELATIONSHIPS_H__

#include <stdint.h>
#include "xlsxwriter/third_party/queue.h"

#include "common.h"

#define LXW_SCHEMA_ROOT     "http://schemas.openxmlformats.org"
#define LXW_PACKAGE_SCHEMA  LXW_SCHEMA_ROOT "/package/2006/relationships"
#define LXW_DOCUMENT_SCHEMA LXW_SCHEMA_ROOT "/officeDocument/2006/relationships"
#define LXW_MS_SCHEMA       "http://schemas.microsoft.com/office/2006/relationships"

/* Define the queue.h STAILQ structs for the generic data structs. */
STAILQ_HEAD(lxw_rel_tuples, lxw_rel_tuple);

typedef struct lxw_rel_tuple {

    char *type;
    char *target;
    char *target_mode;

    STAILQ_ENTRY (lxw_rel_tuple) list_pointers;

} lxw_rel_tuple;

/*
 * Struct to represent a relationships.
 */
typedef struct lxw_relationships {

    FILE *file;

    uint32_t rel_id;
    struct lxw_rel_tuples *relationships;

} lxw_relationships;



/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

lxw_relationships *_new_relationships();
void _free_relationships(lxw_relationships *relationships);
void _relationships_assemble_xml_file(lxw_relationships *self);

void _add_document_relationship(lxw_relationships *self, const char *type,
                                const char *target);
void _add_package_relationship(lxw_relationships *self, const char *type,
                               const char *target);
void _add_ms_package_relationship(lxw_relationships *self, const char *type,
                                  const char *target);
void _add_worksheet_relationship(lxw_relationships *self, const char *type,
                                 const char *target, const char *target_mode);

/* Declarations required for unit testing. */
#ifdef TESTING

STATIC void _relationships_xml_declaration(lxw_relationships *self);

#endif /* TESTING */

/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */

#endif /* __LXW_RELATIONSHIPS_H__ */
