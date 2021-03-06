/* $Id$ */

/* Please see the LICENSE file for copyright and distribution information */

#ifndef __RXML_DOCUMENT__
#define __RXML_DOCUMENT__

extern VALUE cXMLDocument;

void ruby_init_xml_document();


#if defined(_WIN32)
__declspec(dllexport) 
#endif
VALUE rxml_document_wrap(xmlDocPtr xnode);
#endif
