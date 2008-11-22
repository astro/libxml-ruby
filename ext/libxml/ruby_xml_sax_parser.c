/* $Id$ */

/* Please see the LICENSE file for copyright and distribution information */

#include "ruby_libxml.h"
#include "ruby_xml_sax_parser.h"

/*
 * Document-class: LibXML::XML::SaxParser
 *
 * XML::SaxParser provides a callback based API for parsing documents,
 * in contrast to XML::Parser's tree based API and XML::Reader's stream
 * based API.  
 *
 * Note that the XML::SaxParser API is fairly complex, not well standardized, 
 * and does not directly support validation making entity, namespace and
 * base processing relatively hard.
 *
 * To use the XML::SaxParser, register a callback class via the
 * XML::SaxParser#callbacks=.  It is easiest to include the 
 * XML::SaxParser::Callbacks module in your class and override 
 * the methods as needed.
 *
 * Basic example:
 *
 *   class MyCallbacks
 *     include XML::SaxParser::Callbacks
 *     def on_start_element(element, attributes)
 *       puts #Element started: #{element}"
 *     end
 *   end
 *
 *   parser = XML::SaxParser.new
 *   parser.callbacks = MyCallbacks.new
 *   parser.parse
 */

VALUE cXMLSaxParser;
VALUE mXMLSaxParserCallbacks;

static ID INPUT_ATTR;
static ID CALLBACKS_ATTR;

VALUE cbidOnInternalSubset;
VALUE cbidOnIsStandalone;
VALUE cbidOnHasInternalSubset;
VALUE cbidOnHasExternalSubset;
VALUE cbidOnStartDocument;
VALUE cbidOnEndDocument;
VALUE cbidOnStartElement;
VALUE cbidOnEndElement;
VALUE cbidOnReference;
VALUE cbidOnCharacters;
VALUE cbidOnProcessingInstruction;
VALUE cbidOnComment;
VALUE cbidOnXmlParserWarning;
VALUE cbidOnXmlParserError;
VALUE cbidOnXmlParserFatalError;
VALUE cbidOnCdataBlock;
VALUE cbidOnExternalSubset;

#include "sax_parser_callbacks.inc"

/*
 * call-seq:
 *    sax_parser.initialize -> sax_parser
 * 
 * Initiliazes instance of parser.
 */
static VALUE
rxml_sax_parser_initialize(VALUE self) {
  VALUE input = rb_class_new_instance(0, NULL, cXMLInput);
  rb_iv_set(self, "@input", input);
  return self;
}


/* Parsing data sources */
static int
rxml_sax_parser_parse_file(VALUE self, VALUE input) {
  VALUE file = rb_ivar_get(input, FILE_ATTR);
  return xmlSAXUserParseFile(&rxml_sax_hander_struct, self, StringValuePtr(file));
}

static int
rxml_sax_parser_parse_string(VALUE self, VALUE input) {
  VALUE str = rb_ivar_get(input, STRING_ATTR);
  return xmlSAXUserParseMemory(&rxml_sax_hander_struct, self, StringValuePtr(str), RSTRING_LEN(str));
}

static int
rxml_sax_parser_parse_io(VALUE self, VALUE input) {
  VALUE io = rb_ivar_get(input, IO_ATTR);
  VALUE encoding = rb_ivar_get(input, ENCODING_ATTR);
  xmlCharEncoding xmlEncoding = NUM2INT(encoding);
  xmlParserCtxtPtr ctxt = xmlCreateIOParserCtxt(&rxml_sax_hander_struct, self,
	 			                                        (xmlInputReadCallback) rxml_read_callback,
				                                         NULL, io, xmlEncoding);
  return xmlParseDocument(ctxt);
}


/*
 * call-seq:
 *    parser.parse -> (true|false)
 * 
 * Parse the input XML, generating callbacks to the object
 * registered via the +callbacks+ attributesibute.
 */
static VALUE
rxml_sax_parser_parse(VALUE self) {
  int status;
  VALUE input = rb_ivar_get(self, INPUT_ATTR);
  
  if (rb_ivar_get(input, FILE_ATTR) != Qnil)
    status = rxml_sax_parser_parse_file(self, input);
  else if (rb_ivar_get(input, STRING_ATTR) != Qnil)
    status = rxml_sax_parser_parse_string(self, input);
  else if (rb_ivar_get(input, IO_ATTR) != Qnil)
    status = rxml_sax_parser_parse_io(self, input);
  else
    rb_raise(rb_eArgError, "You must specify a parser data source");
  
  if (status)
  {
    rxml_raise(&xmlLastError);
    return Qfalse;
  }
  else
  {
    return(Qtrue);
  }
}

// Rdoc needs to know 
#ifdef RDOC_NEVER_DEFINED
  mLibXML = rb_define_module("LibXML");
  mXML = rb_define_module_under(mLibXML, "XML");
#endif

void
ruby_init_xml_sax_parser(void) {
  /* SaxParser */
  cXMLSaxParser = rb_define_class_under(mXML, "SaxParser", rb_cObject);

  /* Atributes */
  CALLBACKS_ATTR = rb_intern("@callbacks");
  INPUT_ATTR = rb_intern("@input");
  rb_define_attr(cXMLSaxParser, "callbacks", 1, 1);
  rb_define_attr(cXMLSaxParser, "input", 1, 0);

  /* Instance Methods */
  rb_define_method(cXMLSaxParser, "initialize", rxml_sax_parser_initialize, 0);
  rb_define_method(cXMLSaxParser, "parse", rxml_sax_parser_parse, 0);

  /* SaxCallbacks */
  cbidOnInternalSubset = rb_intern("on_internal_subset");
  cbidOnIsStandalone = rb_intern("on_is_standalone");
  cbidOnHasInternalSubset = rb_intern("on_has_internal_subset");
  cbidOnHasExternalSubset = rb_intern("on_has_external_subset");
  cbidOnStartDocument = rb_intern("on_start_document");
  cbidOnEndDocument = rb_intern("on_end_document");
  cbidOnStartElement = rb_intern("on_start_element");
  cbidOnEndElement = rb_intern("on_end_element");
  cbidOnReference = rb_intern("on_reference");
  cbidOnCharacters = rb_intern("on_characters");
  cbidOnProcessingInstruction = rb_intern("on_processing_instruction");
  cbidOnComment = rb_intern("on_comment");
  cbidOnXmlParserWarning = rb_intern("on_parser_warning");
  cbidOnXmlParserError = rb_intern("on_parser_error");
  cbidOnXmlParserFatalError = rb_intern("on_parser_fatal_error");
  cbidOnCdataBlock = rb_intern("on_cdata_block");
  cbidOnExternalSubset = rb_intern("on_external_subset");
}
