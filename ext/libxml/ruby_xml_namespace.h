/* $Id$ */

/* Please see the LICENSE file for copyright and distribution information */

#ifndef __RXML_NAMESPACE__
#define __RXML_NAMESPACE__

extern VALUE cXMLNamespace;

void ruby_init_xml_namespace(void);
VALUE rxml_namespace_wrap(xmlNsPtr ns);
#endif
