/* $Id: ruby_xml_parser_context.c 710 2009-01-20 05:30:51Z cfis $ */

/* Please see the LICENSE file for copyright and distribution information */

#include "ruby_libxml.h"
#include "ruby_xml_html_parser_context.h"

/*
 * Document-class: LibXML::XML::HTMLParser::Context
 *
 * The XML::HTMLParser::Context class provides in-depth control over how
 * a document is parsed.
 */

VALUE cXMLHtmlParserContext;

static void rxml_html_parser_context_free(xmlParserCtxtPtr ctxt)
{
  xmlFreeParserCtxt(ctxt);
}

static VALUE rxml_html_parser_context_wrap(xmlParserCtxtPtr ctxt)
{
  return Data_Wrap_Struct(cXMLHtmlParserContext, NULL, rxml_html_parser_context_free, ctxt);
}

/* call-seq:
 *    XML::HTMLParser::Context.file(file) -> XML::HTMLParser::Context
 *
 * Creates a new parser context based on the specified file or uri.
 *
 * Parameters:
 *
 *  file - A filename or uri.
*/
static VALUE rxml_html_parser_context_file(VALUE klass, VALUE file)
{
  xmlParserCtxtPtr ctxt = htmlCreateFileParserCtxt(StringValuePtr(file), NULL);
  return rxml_html_parser_context_wrap(ctxt);
}

/* call-seq:
 *    XML::HTMLParser::Context.io(io) -> XML::HTMLParser::Context
 *
 * Creates a new parser context based on the specified io object.
 *
 * Parameters:
 *
 *  io - A ruby IO object.
*/
static VALUE rxml_html_parser_context_io(VALUE klass, VALUE io)
{
  htmlParserCtxtPtr ctxt;
  xmlParserInputBufferPtr input;
  xmlParserInputPtr stream;

  input = xmlParserInputBufferCreateIO((xmlInputReadCallback) rxml_read_callback, NULL,
                                     (void*)io, XML_CHAR_ENCODING_NONE);

  ctxt = htmlNewParserCtxt();
  if (!ctxt)
  {
    xmlFreeParserInputBuffer(input);
    rxml_raise(&xmlLastError);
  }

  stream = xmlNewIOInputStream(ctxt, input, XML_CHAR_ENCODING_NONE);

  if (!stream)
  {
    xmlFreeParserInputBuffer(input);
    xmlFreeParserCtxt(ctxt);
    rxml_raise(&xmlLastError);
  }
  inputPush(ctxt, stream);

  return rxml_html_parser_context_wrap(ctxt);
}

/* call-seq:
 *    XML::HTMLParser::Context.string(string) -> XML::HTMLParser::Context
 *
 * Creates a new parser context based on the specified string.
 *
 * Parameters:
 *
 *  string - A string that contains the data to parse.
*/
static VALUE rxml_html_parser_context_string(VALUE klass, VALUE string)
{
  xmlParserCtxtPtr ctxt;
  Check_Type(string, T_STRING);

  if (RSTRING_LEN(string) == 0)
    rb_raise(rb_eArgError, "Must specify a string with one or more characters");

  ctxt = xmlCreateMemoryParserCtxt(StringValuePtr(string),
                                   RSTRING_LEN(string));
  if (!ctxt)
    rxml_raise(&xmlLastError);

  htmlDefaultSAXHandlerInit();
  if (ctxt->sax != NULL)
    memcpy(ctxt->sax, &htmlDefaultSAXHandler, sizeof(xmlSAXHandlerV1));
  
  return rxml_html_parser_context_wrap(ctxt);
}

/*
 * call-seq:
 *    context.options = XML::Parser::Options::NOENT |
                        XML::Parser::Options::NOCDATA
 *
 * Provides control over the execution of a parser.  Valid values 
 * are the constants defined on XML::Parser::Options.  Multiple
 * options can be combined by using Bitwise OR (|).
 */
static VALUE rxml_html_parser_context_options_set(VALUE self, VALUE options)
{
  int result;
  xmlParserCtxtPtr ctxt;
  Check_Type(options, T_FIXNUM);

  Data_Get_Struct(self, xmlParserCtxt, ctxt);
  result = htmlCtxtUseOptions(ctxt, NUM2INT(options));

  return self;
}

// Rdoc needs to know 
#ifdef RDOC_NEVER_DEFINED
  mLibXML = rb_define_module("LibXML");
  mXML = rb_define_module_under(mLibXML, "XML");
  cXMLHtmlParser = rb_define_class_under(mXML, "HtmlParser", rb_cObject);
#endif

void ruby_init_html_parser_context(void)
{
  cXMLHtmlParserContext = rb_define_class_under(cXMLHtmlParser, "Context", cXMLParserContext);

  rb_define_singleton_method(cXMLHtmlParserContext, "file", rxml_html_parser_context_file, 1);
  rb_define_singleton_method(cXMLHtmlParserContext, "io", rxml_html_parser_context_io, 1);
  rb_define_singleton_method(cXMLHtmlParserContext, "string", rxml_html_parser_context_string, 1);
  rb_define_method(cXMLHtmlParserContext, "options=", rxml_html_parser_context_options_set, 1);
}
