/* Please see the LICENSE file for copyright and distribution information */

#ifndef __RUBY_LIBXML_H__
#define __RUBY_LIBXML_H__

#include "version.h"

#include <ruby.h>
#include <rubyio.h>
#include <util.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/debugXML.h>
#include <libxml/xmlversion.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>
#include <libxml/valid.h>
#include <libxml/catalog.h>
#include <libxml/HTMLparser.h>
#include <libxml/xmlreader.h>

// Maybe not yet defined in ruby
#ifndef RSTRING_LEN
#define RSTRING_LEN(x) RSTRING(x)->len
#endif

// not in Ruby 1.9
#ifndef GetWriteFile
#define GetWriteFile(fp) rb_io_stdio_file(fp)
#define OpenFile rb_io_t
#endif

#ifdef LIBXML_DEBUG_ENABLED
#include <libxml/xpathInternals.h>
#endif
#ifdef LIBXML_XINCLUDE_ENABLED
#include <libxml/xinclude.h>
#endif
#ifdef LIBXML_XPTR_ENABLED
#include <libxml/xpointer.h>
#endif

#include "ruby_xml_error.h"
#include "ruby_xml_input.h"
#include "ruby_xml_state.h"
#include "ruby_xml_attributes.h"
#include "ruby_xml_attr.h"
#include "ruby_xml_document.h"
#include "ruby_xml_node.h"
#include "ruby_xml_ns.h"
#include "ruby_xml_parser.h"
#include "ruby_xml_parser_context.h"
#include "ruby_xml_sax_parser.h"
#include "ruby_xml_xinclude.h"
#include "ruby_xml_xpath.h"
#include "ruby_xml_xpath_expression.h"
#include "ruby_xml_xpath_context.h"
#include "ruby_xml_xpath_object.h"
#include "ruby_xml_xpointer.h"
#include "ruby_xml_input_cbg.h"
#include "ruby_xml_dtd.h"
#include "ruby_xml_schema.h"
#include "ruby_xml_relaxng.h"
#include "ruby_xml_html_parser.h"
#include "ruby_xml_reader.h"

extern VALUE mLibXML;
extern VALUE mXML;

#endif
