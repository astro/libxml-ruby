/* $Id$ */

/* Please see the LICENSE file for copyright and distribution information */

#ifndef __RXML_NODE__
#define __RXML_NODE__

extern VALUE cXMLNode;

void ruby_init_xml_node(void);
void rxml_node_mark_common(xmlNodePtr xnode);
VALUE rxml_node_wrap(xmlNodePtr xnode);
VALUE check_string_or_symbol(VALUE val);
#endif
