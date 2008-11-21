/* Copyright (c) 2006 Apple Computer Inc.
 * Please see the LICENSE file for copyright and distribution information. */

#ifndef __rxml_READER__
#define __rxml_READER__

extern VALUE cXMLReader;

void ruby_init_xml_reader(void);

/* Exported to be used by XML::Document#reader */
VALUE rxml_reader_new_walker(VALUE self, VALUE doc);

#endif /* __rxml_READER__ */
