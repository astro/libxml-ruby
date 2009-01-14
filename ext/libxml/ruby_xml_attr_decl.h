/* $Id: ruby_xml_attr.h 666 2008-12-07 00:16:50Z cfis $ */

/* Please see the LICENSE file for copyright and distribution information */

#ifndef __RXML_ATTR_DECL__
#define __RXML_ATTR_DECL__

extern VALUE cXMLAttrDecl;

void ruby_init_xml_attr_decl(void);
VALUE rxml_attr_decl_wrap(xmlAttributePtr xattribute);
VALUE rxml_attr_decl_value_get(VALUE self);
#endif
