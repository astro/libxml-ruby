/* Copyright (c) 2006-2007 Apple Inc.
 * Please see the LICENSE file for copyright and distribution information. */

#include "ruby_libxml.h"
#include "ruby_xml_reader.h"

/*
 * Document-class: LibXML::XML::Reader
 *
 * The XML::Reader class provides a simpler, alternative way of parsing an XML
 * document in contrast to XML::Parser or XML::SaxParser.  A XML::Reader instance
 * acts like a cursor going forward in a document stream, stopping at each node
 * it encounters.  To advance to the next node, simply cadd XML::Reader#read.
 *
 * The XML::Reader API closely matches the DOM Core specification and supports
 * namespaces, xml:base, entity handling and DTDs.
 *
 * To summarize, XML::Reader provides a far simpler API to use versus XML::SaxParser
 * and is more memory efficient than using XML::Parser to create a DOM tree.
 *
 * Example:
 *
 *  parser = XML::Reader.string("<foo><bar>1</bar><bar>2</bar><bar>3</bar></foo>")
 *  reader.read
 *  assert_equal('foo', reader.name)
 *  assert_equal(nil, reader.value)
 *
 *  3.times do |i|
 *    reader.read
 *    assert_equal(XML::Reader::TYPE_ELEMENT, reader.node_type)
 *    assert_equal('bar', reader.name)
 *    reader.read
 *    assert_equal(XML::Reader::TYPE_TEXT, reader.node_type)
 *    assert_equal((i + 1).to_s, reader.value)
 *    reader.read
 *    assert_equal(XML::Reader::TYPE_END_ELEMENT, reader.node_type)
 *  end
 *
 * You can also parse documents (see XML::Reader.document), 
 * strings (see XML::Parser.string) and io objects (see
 * XML::Parser.io).
 *
 * For a more in depth tutorial, albeit in C, see http://xmlsoft.org/xmlreader.html.*/

VALUE cXMLReader;

ID base_uri_SYMBOL;
ID ENCODING_SYMBOL;
ID OPTIONS_SYMBOL;

static VALUE rxml_reader_wrap(xmlTextReaderPtr reader)
{
  return Data_Wrap_Struct(cXMLReader, NULL, xmlFreeTextReader, reader);
}

static xmlTextReaderPtr rxml_text_reader_get(VALUE obj)
{
  xmlTextReaderPtr xreader;
  Data_Get_Struct(obj, xmlTextReader, xreader);
  return xreader;
}

/*
 * call-seq:
 *    XML::Reader.document(doc) -> XML::Reader
 *
 * Create an new reader for the specified document.
 */
VALUE rxml_reader_document(VALUE klass, VALUE doc)
{
  xmlDocPtr xdoc;
  xmlTextReaderPtr xreader;

  Data_Get_Struct(doc, xmlDoc, xdoc);

  xreader = xmlReaderWalker(xdoc);

  if (xreader == NULL)
    rxml_raise(&xmlLastError);

  return rxml_reader_wrap(xreader);
}

/* call-seq:
 *    XML::Reader.file(path) -> XML::Reader
 *    XML::Reader.file(path, :encoding => XML::Encoding::UTF_8,
 *                           :options => XML::Parser::Options::NOENT) -> XML::Parser
 *
 * Creates a new reader by parsing the specified file or uri.
 *
 * You may provide an optional hash table to control how the
 * parsing is performed.  Valid options are:
 *
 *  encoding - The document encoding, defaults to nil. Valid values
 *             are the encoding constants defined on XML::Encoding.
 *  options - Controls the execution of the parser, defaults to 0.
 *            Valid values are the constants defined on
 *            XML::Parser::Options.  Mutliple options can be combined
 *            by using Bitwise OR (|). 
 */
static VALUE rxml_reader_file(int argc, VALUE *argv, VALUE klass)
{
  xmlTextReaderPtr xreader;
  VALUE path;
  VALUE options;

  const char *xencoding = NULL;
  int xoptions = 0;

  rb_scan_args(argc, argv, "11", &path, &options);
  Check_Type(path, T_STRING);

  if (!NIL_P(options))
  {
    VALUE encoding = Qnil;
    VALUE parserOptions = Qnil;

    Check_Type(options, T_HASH);

    encoding = rb_hash_aref(options, base_uri_SYMBOL);
    xencoding = NIL_P(encoding) ? NULL : xmlGetCharEncodingName(NUM2INT(encoding));

    parserOptions = rb_hash_aref(options, OPTIONS_SYMBOL);
    xoptions = NIL_P(parserOptions) ? 0 : NUM2INT(parserOptions);
  }

  xreader = xmlReaderForFile(StringValueCStr(path), xencoding, xoptions);

  if (xreader == NULL)
    rxml_raise(&xmlLastError);

  return rxml_reader_wrap(xreader);
}

/* call-seq:
 *    XML::Reader.io(io) -> XML::Reader
 *    XML::Reader.io(io, :encoding => XML::Encoding::UTF_8,
 *                       :options => XML::Parser::Options::NOENT) -> XML::Parser
 *
 * Creates a new reader by parsing the specified io object.
 *
 * You may provide an optional hash table to control how the
 * parsing is performed.  Valid options are:
 *
 *  base_uri - The base url for the parsed document.
 *  encoding - The document encoding, defaults to nil. Valid values
 *             are the encoding constants defined on XML::Encoding.
 *  options - Controls the execution of the parser, defaults to 0.
 *            Valid values are the constants defined on
 *            XML::Parser::Options.  Mutliple options can be combined
 *            by using Bitwise OR (|). 
 */
static VALUE rxml_reader_io(int argc, VALUE *argv, VALUE klass)
{
  xmlTextReaderPtr xreader;
  VALUE io;
  VALUE options;
  char *xbaseurl = NULL;
  const char *xencoding = NULL;
  int xoptions = 0;

  rb_scan_args(argc, argv, "11", &io, &options);

  if (!NIL_P(options))
  {
    VALUE baseurl = Qnil;
    VALUE encoding = Qnil;
    VALUE parserOptions = Qnil;

    Check_Type(options, T_HASH);

    baseurl = rb_hash_aref(options, base_uri_SYMBOL);
    xbaseurl = NIL_P(baseurl) ? NULL : StringValueCStr(baseurl);

    encoding = rb_hash_aref(options, ENCODING_SYMBOL);
    xencoding = NIL_P(encoding) ? NULL : xmlGetCharEncodingName(NUM2INT(encoding));

    parserOptions = rb_hash_aref(options, OPTIONS_SYMBOL);
    xoptions = NIL_P(parserOptions) ? 0 : NUM2INT(parserOptions);
  }
  
  xreader = xmlReaderForIO((xmlInputReadCallback) rxml_read_callback, NULL,
                           (void *) io, 
                           xbaseurl, xencoding, xoptions);

  if (xreader == NULL)
    rxml_raise(&xmlLastError);

  return rxml_reader_wrap(xreader);
}

/* call-seq:
 *    XML::Reader.string(io) -> XML::Reader
 *    XML::Reader.string(io, :encoding => XML::Encoding::UTF_8,
 *                           :options => XML::Parser::Options::NOENT) -> XML::Parser
 *
 * Creates a new reader by parsing the specified string.
 *
 * You may provide an optional hash table to control how the
 * parsing is performed.  Valid options are:
 *
 *  base_uri - The base url for the parsed document.
 *  encoding - The document encoding, defaults to nil. Valid values
 *             are the encoding constants defined on XML::Encoding.
 *  options - Controls the execution of the parser, defaults to 0.
 *            Valid values are the constants defined on
 *            XML::Parser::Options.  Mutliple options can be combined
 *            by using Bitwise OR (|).
 */
static VALUE rxml_reader_string(int argc, VALUE *argv, VALUE klass)
{
  xmlTextReaderPtr xreader;
  VALUE string;
  VALUE options;
  char *xbaseurl = NULL;
  const char *xencoding = NULL;
  int xoptions = 0;

  rb_scan_args(argc, argv, "11", &string, &options);
  Check_Type(string, T_STRING);

  if (!NIL_P(options))
  {
    VALUE baseurl = Qnil;
    VALUE encoding = Qnil;
    VALUE parserOptions = Qnil;

    Check_Type(options, T_HASH);

    baseurl = rb_hash_aref(options, base_uri_SYMBOL);
    xbaseurl = NIL_P(baseurl) ? NULL : StringValueCStr(baseurl);

    encoding = rb_hash_aref(options, ENCODING_SYMBOL);
    xencoding = NIL_P(encoding) ? NULL : xmlGetCharEncodingName(NUM2INT(encoding));
      
    parserOptions = rb_hash_aref(options, OPTIONS_SYMBOL);
    xoptions = NIL_P(parserOptions) ? 0 : NUM2INT(parserOptions);
  }
  
  xreader = xmlReaderForMemory(StringValueCStr(string), RSTRING_LEN(string), 
                               xbaseurl, xencoding, xoptions);

  if (xreader == NULL)
    rxml_raise(&xmlLastError);

  return rxml_reader_wrap(xreader);
}

/*
 * call-seq:
 *    reader.close -> code
 *
 * This method releases any resources allocated by the current instance
 * changes the state to Closed and close any underlying input.
 */
static VALUE rxml_reader_close(VALUE self)
{
  return INT2FIX(xmlTextReaderClose(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *   reader.move_to_attribute(val) -> code
 *
 * Move the position of the current instance to the attribute with the
 * specified index (if +val+ is an integer) or name (if +val+ is a string)
 * relative to the containing element.
 */
static VALUE rxml_reader_move_to_attr(VALUE self, VALUE val)
{
  xmlTextReaderPtr xreader;
  int ret;

  xreader = rxml_text_reader_get(self);

  if (TYPE(val) == T_FIXNUM)
  {
    ret = xmlTextReaderMoveToAttributeNo(xreader, FIX2INT(val));
  }
  else
  {
    ret = xmlTextReaderMoveToAttribute(xreader,
        (const xmlChar *) StringValueCStr(val));
  }

  return INT2FIX(ret);
}

/*
 * call-seq:
 *    reader.move_to_first_attribute -> code
 *
 * Move the position of the current instance to the first attribute associated
 * with the current node.
 */
static VALUE rxml_reader_move_to_first_attr(VALUE self)
{
  return INT2FIX(xmlTextReaderMoveToFirstAttribute(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.move_to_next_attribute -> code
 *
 * Move the position of the current instance to the next attribute associated
 * with the current node.
 */
static VALUE rxml_reader_move_to_next_attr(VALUE self)
{
  return INT2FIX(xmlTextReaderMoveToNextAttribute(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.move_to_element -> code
 *
 * Move the position of the current instance to the node that contains the
 * current attribute node.
 */
static VALUE rxml_reader_move_to_element(VALUE self)
{
  return INT2FIX(xmlTextReaderMoveToElement(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.next -> code
 *
 * Skip to the node following the current one in document order while avoiding
 * the subtree if any.
 */
static VALUE rxml_reader_next(VALUE self)
{
  return INT2FIX(xmlTextReaderNext(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.next_sibling -> code
 *
 * Skip to the node following the current one in document order while avoiding
 * the subtree if any. Currently implemented only for Readers built on a
 * document.
 */
static VALUE rxml_reader_next_sibling(VALUE self)
{
  return INT2FIX(xmlTextReaderNextSibling(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.node -> XML::Node
 *
 * Returns the reader's current node.
 * WARNING - Using this method is dangerous because the
 * the node may be destroyed on the next #read.
 */
static VALUE rxml_reader_node(VALUE self)
{
  xmlTextReaderPtr xreader = rxml_text_reader_get(self);
  xmlNodePtr xnode = xmlTextReaderCurrentNode(xreader);
  return rxml_node_wrap(xnode);
}

/*
 * call-seq:
 *    reader.node_type -> type
 *
 * Get the node type of the current node. Reference:
 * http://dotgnu.org/pnetlib-doc/System/Xml/XmlNodeType.html
 */
static VALUE rxml_reader_node_type(VALUE self)
{
  return INT2FIX(xmlTextReaderNodeType(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.normalization -> value
 *
 * The value indicating whether to normalize white space and attribute values.
 * Since attribute value and end of line normalizations are a MUST in the XML
 * specification only the value true is accepted. The broken bahaviour of
 * accepting out of range character entities like &#0; is of course not
 * supported either.
 *
 * Return 1 or -1 in case of error.
 */
static VALUE rxml_reader_normalization(VALUE self)
{
  return INT2FIX(xmlTextReaderNormalization(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.read -> code
 *
 * Causes the reader to move to the next node in the stream, exposing its properties.
 *
 * Returns true if a node was successfully read or false if there are no more
 * nodes to read.  On errors, an exception is raised.*/
static VALUE rxml_reader_read(VALUE self)
{
  int result = xmlTextReaderRead(rxml_text_reader_get(self));
  switch(result)
  {
    case -1:
      rxml_raise(&xmlLastError);
      return Qnil;
      break;
    case 0:
      return Qfalse;
    case 1:
      return Qtrue;
    default:
      rb_raise(rb_eRuntimeError,
               "xmlTextReaderRead did not return -1, 0 or 1.  Return value was: %d", result);
  }
}

/*
 * call-seq:
 *    reader.read_attribute_value -> code
 *
 * Parse an attribute value into one or more Text and EntityReference nodes.
 *
 * Return 1 in case of success, 0 if the reader was not positionned on an
 * attribute node or all the attribute values have been read, or -1 in case of
 * error.
 */
static VALUE rxml_reader_read_attr_value(VALUE self)
{
  return INT2FIX(xmlTextReaderReadAttributeValue(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.read_inner_xml -> data
 *
 * Read the contents of the current node, including child nodes and markup.
 *
 * Return a string containing the XML content, or nil if the current node is
 * neither an element nor attribute, or has no child nodes.
 */
static VALUE rxml_reader_read_inner_xml(VALUE self)
{
  const xmlChar *result = xmlTextReaderReadInnerXml(rxml_text_reader_get(self));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.read_outer_xml -> data
 *
 * Read the contents of the current node, including child nodes and markup.
 *
 * Return a string containing the XML content, or nil if the current node is
 * neither an element nor attribute, or has no child nodes.
 */
static VALUE rxml_reader_read_outer_xml(VALUE self)
{
  const xmlChar *result = xmlTextReaderReadOuterXml(rxml_text_reader_get(self));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.read_state -> state
 *
 * Get the read state of the reader.
 */
static VALUE rxml_reader_read_state(VALUE self)
{
  return INT2FIX(xmlTextReaderReadState(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.read_string -> string
 *
 * Read the contents of an element or a text node as a string.
 *
 * Return a string containing the contents of the Element or Text node, or nil
 * if the reader is positioned on any other type of node.
 */
static VALUE rxml_reader_read_string(VALUE self)
{
  const xmlChar *result = xmlTextReaderReadString(rxml_text_reader_get(self));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.relax_ng_validate(rng) -> code
 *
 * Use RelaxNG to validate the document as it is processed. Activation is only
 * possible before the first read. If +rng+ is nil, the RelaxNG validation is
 * desactivated.
 *
 * Return 0 in case the RelaxNG validation could be (des)activated and -1 in
 * case of error.
 */
static VALUE rxml_reader_relax_ng_validate(VALUE self, VALUE rng)
{
  char *xrng = NIL_P(rng) ? NULL : StringValueCStr(rng);
  return INT2FIX(xmlTextReaderRelaxNGValidate(rxml_text_reader_get(self), xrng));
}

#if LIBXML_VERSION >= 20620
/*
 * call-seq:
 *    reader.schema_validate(schema) -> code
 *
 * Use W3C XSD schema to validate the document as it is processed. Activation
 * is only possible before the first read. If +schema+ is nil, then XML Schema
 * validation is desactivated.
 *
 * Return 0 in case the schemas validation could be (de)activated and -1 in
 * case of error.
 */
static VALUE
rxml_reader_schema_validate(VALUE self, VALUE xsd)
{
  char *xxsd = NIL_P(xsd) ? NULL : StringValueCStr(xsd);
  int status = xmlTextReaderSchemaValidate(rxml_text_reader_get(self), xxsd);
  return INT2FIX(status);
}
#endif

/*
 * call-seq:
 *    reader.name -> name
 *
 * Return the qualified name of the node.
 */
static VALUE rxml_reader_name(VALUE self)
{
  const xmlChar *result = xmlTextReaderConstName(rxml_text_reader_get(self));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.local_name -> name
 *
 * Return the local name of the node.
 */
static VALUE rxml_reader_local_name(VALUE self)
{
  const xmlChar *result = xmlTextReaderConstLocalName(rxml_text_reader_get(self));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.attribute_count -> count
 *
 * Provide the number of attributes of the current node.
 */
static VALUE rxml_reader_attr_count(VALUE self)
{
  return INT2FIX(xmlTextReaderAttributeCount(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.encoding -> XML::Encoding::UTF_8
 *
 * Returns the encoding of the document being read.  Note you
 * first have to read data from the reader for encoding
 * to return a value
 *
 *   reader = XML::Reader.file(XML_FILE)
 *   assert_nil(reader.encoding)
 *   reader.read
 *   assert_equal(XML::Encoding::UTF_8, reader.encoding)
 *
 * In addition, libxml always appears to return nil for the encoding
 * when parsing strings.
 */
static VALUE rxml_reader_encoding(VALUE self)
{
  xmlTextReaderPtr xreader = rxml_text_reader_get(self);
  const xmlChar *xencoding = xmlTextReaderConstEncoding(xreader);
  if (xencoding)
    return INT2NUM(xmlParseCharEncoding(xencoding));
  else
    return INT2NUM(XML_CHAR_ENCODING_NONE);
}

/*
 * call-seq:
 *    reader.base_uri -> URI
 *
 * Determine the base URI of the node.
 */
static VALUE rxml_reader_base_uri(VALUE self)
{
  const xmlChar *result = xmlTextReaderConstBaseUri(rxml_text_reader_get(self));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.namespace_uri -> URI
 *
 * Determine the namespace URI of the node.
 */
static VALUE rxml_reader_namespace_uri(VALUE self)
{
  const xmlChar *result = xmlTextReaderConstNamespaceUri(rxml_text_reader_get(self));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.value -> text
 *
 * Provide the text value of the node if present.
 */
static VALUE rxml_reader_value(VALUE self)
{
  const xmlChar *result = xmlTextReaderConstValue(rxml_text_reader_get(self));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.prefix -> prefix
 *
 * Get a shorthand reference to the namespace associated with the node.
 */
static VALUE rxml_reader_prefix(VALUE self)
{
  const xmlChar *result = xmlTextReaderConstPrefix(rxml_text_reader_get(self));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.depth -> depth
 *
 * Get the depth of the node in the tree.
 */
static VALUE rxml_reader_depth(VALUE self)
{
  return INT2FIX(xmlTextReaderDepth(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.quote_char -> char
 *
 * Get the quotation mark character used to enclose the value of an attribute,
 * as an integer value (and -1 in case of error).
 */
static VALUE rxml_reader_quote_char(VALUE self)
{
  return INT2FIX(xmlTextReaderQuoteChar(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.standalone -> code
 *
 * Determine the standalone status of the document being read.
 *
 * Return 1 if the document was declared to be standalone, 0 if it was
 * declared to be not standalone, or -1 if the document did not specify its
 * standalone status or in case of error.
 */
static VALUE rxml_reader_standalone(VALUE self)
{
  return INT2FIX(xmlTextReaderStandalone(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.xml_lang -> value
 *
 * Get the xml:lang scope within which the node resides.
 */
static VALUE rxml_reader_xml_lang(VALUE self)
{
  const xmlChar *result = xmlTextReaderConstXmlLang(rxml_text_reader_get(self));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.xml_version -> version
 *
 * Determine the XML version of the document being read.
 */
static VALUE rxml_reader_xml_version(VALUE self)
{
  const xmlChar *result = xmlTextReaderConstXmlVersion(rxml_text_reader_get(self));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.has_attributes? -> bool
 *
 * Get whether the node has attributes.
 */
static VALUE rxml_reader_has_attributes(VALUE self)
{
  return xmlTextReaderHasAttributes(rxml_text_reader_get(self)) ? Qtrue
      : Qfalse;
}

/*
 * call-seq:
 *    reader.has_value? -> bool
 *
 * Get whether the node can have a text value.
 */
static VALUE rxml_reader_has_value(VALUE self)
{
  return xmlTextReaderHasValue(rxml_text_reader_get(self)) ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *    reader[key] -> value
 *
 * Provide the value of the attribute with the specified index (if +key+ is an
 * integer) or with the specified name (if +key+ is a string) relative to the
 * containing element, as a string.
 */
static VALUE rxml_reader_attribute(VALUE self, VALUE key)
{
  xmlTextReaderPtr reader;
  xmlChar *attr;

  reader = rxml_text_reader_get(self);

  if (TYPE(key) == T_FIXNUM)
  {
    attr = xmlTextReaderGetAttributeNo(reader, FIX2INT(key));
  }
  else
  {
    attr = xmlTextReaderGetAttribute(reader, (const xmlChar *) StringValueCStr(key));
  }
  return (attr == NULL ? Qnil : rb_str_new2((const char*)attr));
}

/*
 * call-seq:
 *    reader.lookup_namespace(prefix) -> value
 *
 * Resolve a namespace prefix in the scope of the current element.
 * To return the default namespace, specify nil as +prefix+.
 */
static VALUE rxml_reader_lookup_namespace(VALUE self, VALUE prefix)
{
  const xmlChar *result = xmlTextReaderLookupNamespace(rxml_text_reader_get(
      self), (const xmlChar *) StringValueCStr(prefix));
  return (result == NULL ? Qnil : rb_str_new2((const char*)result));
}

/*
 * call-seq:
 *    reader.expand -> node
 *
 * Read the contents of the current node and the full subtree. It then makes
 * the subtree available until the next read call.
 *
 * Return an XML::Node object, or nil in case of error.
 */
static VALUE rxml_reader_expand(VALUE self)
{
  xmlNodePtr node;
  xmlDocPtr doc;
  xmlTextReaderPtr reader = rxml_text_reader_get(self);
  node = xmlTextReaderExpand(reader);

  if (!node)
    return Qnil;

  /* Okay this is tricky.  By accessing the returned node, we
   take ownership of the reader's document.  Thus we need to
   tell the reader to not free it.  Otherwise it will be
   freed twice - once when the Ruby document wrapper goes
   out of scope and once when the reader goes out of scope. */

  xmlTextReaderPreserve(reader);
  doc = xmlTextReaderCurrentDoc(reader);
  rxml_document_wrap(doc);

  return rxml_node_wrap(node);
}

#if LIBXML_VERSION >= 20618
/*
 * call-seq:
 *    reader.byte_consumed -> value
 *
 * This method provides the current index of the parser used by the reader,
 * relative to the start of the current entity.
 */
static VALUE
rxml_reader_byte_consumed(VALUE self)
{
  return INT2NUM(xmlTextReaderByteConsumed(rxml_text_reader_get(self)));
}
#endif

#if LIBXML_VERSION >= 20617
/*
 * call-seq:
 *    reader.column_number -> number
 *
 * Provide the column number of the current parsing point.
 */
static VALUE
rxml_reader_column_number(VALUE self)
{
  return INT2NUM(xmlTextReaderGetParserColumnNumber(rxml_text_reader_get(self)));
}

/*
 * call-seq:
 *    reader.line_number -> number
 *
 * Provide the line number of the current parsing point.
 */
static VALUE
rxml_reader_line_number(VALUE self)
{
  return INT2NUM(xmlTextReaderGetParserLineNumber(rxml_text_reader_get(self)));
}
#endif

/*
 * call-seq:
 *    reader.default? -> bool
 *
 * Return whether an Attribute node was generated from the default value
 * defined in the DTD or schema.
 */
static VALUE rxml_reader_default(VALUE self)
{
  return xmlTextReaderIsDefault(rxml_text_reader_get(self)) ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *    reader.namespace_declaration? -> bool
 *
 * Determine whether the current node is a namespace declaration rather than a
 * regular attribute.
 */
static VALUE rxml_reader_namespace_declaration(VALUE self)
{
  return xmlTextReaderIsNamespaceDecl(rxml_text_reader_get(self)) ? Qtrue
      : Qfalse;
}

/*
 * call-seq:
 *    reader.empty_element? -> bool
 *
 * Check if the current node is empty.
 */
static VALUE rxml_reader_empty_element(VALUE self)
{
  return xmlTextReaderIsEmptyElement(rxml_text_reader_get(self)) ? Qtrue
      : Qfalse;
}

/*
 * call-seq:
 *    reader.valid? -> bool
 *
 * Retrieve the validity status from the parser context.
 */
static VALUE rxml_reader_valid(VALUE self)
{
  return xmlTextReaderIsValid(rxml_text_reader_get(self)) ? Qtrue : Qfalse;
}

/* Rdoc needs to know. */
#ifdef RDOC_NEVER_DEFINED
mLibXML = rb_define_module("LibXML");
mXML = rb_define_module_under(mLibXML, "XML");
#endif

void ruby_init_xml_reader(void)
{
  base_uri_SYMBOL = ID2SYM(rb_intern("base_uri"));
  ENCODING_SYMBOL = ID2SYM(rb_intern("encoding"));
  OPTIONS_SYMBOL = ID2SYM(rb_intern("options"));

  cXMLReader = rb_define_class_under(mXML, "Reader", rb_cObject);

  rb_define_singleton_method(cXMLReader, "document", rxml_reader_document, 1);
  rb_define_singleton_method(cXMLReader, "file", rxml_reader_file, -1);
  rb_define_singleton_method(cXMLReader, "io", rxml_reader_io, -1);
  rb_define_singleton_method(cXMLReader, "string", rxml_reader_string, -1);

  rb_define_method(cXMLReader, "[]", rxml_reader_attribute, 1);
  rb_define_method(cXMLReader, "attribute_count", rxml_reader_attr_count, 0);
  rb_define_method(cXMLReader, "base_uri", rxml_reader_base_uri, 0);
#if LIBXML_VERSION >= 20618
  rb_define_method(cXMLReader, "byte_consumed", rxml_reader_byte_consumed, 0);
#endif
  rb_define_method(cXMLReader, "close", rxml_reader_close, 0);
#if LIBXML_VERSION >= 20617
  rb_define_method(cXMLReader, "column_number", rxml_reader_column_number, 0);
#endif
  rb_define_method(cXMLReader, "depth", rxml_reader_depth, 0);
  rb_define_method(cXMLReader, "encoding", rxml_reader_encoding, 0);
  rb_define_method(cXMLReader, "expand", rxml_reader_expand, 0);
  rb_define_method(cXMLReader, "has_attributes?", rxml_reader_has_attributes, 0);
  rb_define_method(cXMLReader, "has_value?", rxml_reader_has_value, 0);
#if LIBXML_VERSION >= 20617
  rb_define_method(cXMLReader, "line_number", rxml_reader_line_number, 0);
#endif
  rb_define_method(cXMLReader, "local_name", rxml_reader_local_name, 0);
  rb_define_method(cXMLReader, "lookup_namespace",       rxml_reader_lookup_namespace, 1);
  rb_define_method(cXMLReader, "move_to_attribute", rxml_reader_move_to_attr, 1);
  rb_define_method(cXMLReader, "move_to_first_attribute",       rxml_reader_move_to_first_attr, 0);
  rb_define_method(cXMLReader, "move_to_next_attribute",       rxml_reader_move_to_next_attr, 0);
  rb_define_method(cXMLReader, "move_to_element", rxml_reader_move_to_element,       0);
  rb_define_method(cXMLReader, "name", rxml_reader_name, 0);
  rb_define_method(cXMLReader, "namespace_uri", rxml_reader_namespace_uri, 0);
  rb_define_method(cXMLReader, "next", rxml_reader_next, 0);
  rb_define_method(cXMLReader, "next_sibling", rxml_reader_next_sibling, 0);
  rb_define_method(cXMLReader, "node", rxml_reader_node, 0);
  rb_define_method(cXMLReader, "node_type", rxml_reader_node_type, 0);
  rb_define_method(cXMLReader, "normalization", rxml_reader_normalization, 0);
  rb_define_method(cXMLReader, "prefix", rxml_reader_prefix, 0);
  rb_define_method(cXMLReader, "quote_char", rxml_reader_quote_char, 0);
  rb_define_method(cXMLReader, "read", rxml_reader_read, 0);
  rb_define_method(cXMLReader, "read_attribute_value", rxml_reader_read_attr_value, 0);
  rb_define_method(cXMLReader, "read_inner_xml", rxml_reader_read_inner_xml, 0);
  rb_define_method(cXMLReader, "read_outer_xml", rxml_reader_read_outer_xml, 0);
  rb_define_method(cXMLReader, "read_state", rxml_reader_read_state, 0);
  rb_define_method(cXMLReader, "read_string", rxml_reader_read_string, 0);
  rb_define_method(cXMLReader, "relax_ng_validate",       rxml_reader_relax_ng_validate, 1);
  rb_define_method(cXMLReader, "standalone", rxml_reader_standalone, 0);
#if LIBXML_VERSION >= 20620
  rb_define_method(cXMLReader, "schema_validate", rxml_reader_schema_validate, 1);
#endif
  rb_define_method(cXMLReader, "value", rxml_reader_value, 0);
  rb_define_method(cXMLReader, "xml_lang", rxml_reader_xml_lang, 0);
  rb_define_method(cXMLReader, "xml_version", rxml_reader_xml_version, 0);
  rb_define_method(cXMLReader, "default?", rxml_reader_default, 0);
  rb_define_method(cXMLReader, "empty_element?", rxml_reader_empty_element, 0);
  rb_define_method(cXMLReader, "namespace_declaration?", rxml_reader_namespace_declaration, 0);
  rb_define_method(cXMLReader, "valid?", rxml_reader_valid, 0);

  /* Constants */
  rb_define_const(cXMLReader, "LOADDTD", INT2FIX(XML_PARSER_LOADDTD));
  rb_define_const(cXMLReader, "DEFAULTATTRS", INT2FIX(XML_PARSER_DEFAULTATTRS));
  rb_define_const(cXMLReader, "VALIDATE", INT2FIX(XML_PARSER_VALIDATE));
  rb_define_const(cXMLReader, "SUBST_ENTITIES", INT2FIX(XML_PARSER_SUBST_ENTITIES));

  rb_define_const(cXMLReader, "SEVERITY_VALIDITY_WARNING", INT2FIX(XML_PARSER_SEVERITY_VALIDITY_WARNING));
  rb_define_const(cXMLReader, "SEVERITY_VALIDITY_ERROR", INT2FIX(XML_PARSER_SEVERITY_VALIDITY_ERROR));
  rb_define_const(cXMLReader, "SEVERITY_WARNING", INT2FIX(XML_PARSER_SEVERITY_WARNING));
  rb_define_const(cXMLReader, "SEVERITY_ERROR", INT2FIX(XML_PARSER_SEVERITY_ERROR));

  rb_define_const(cXMLReader, "TYPE_NONE", INT2FIX(XML_READER_TYPE_NONE));
  rb_define_const(cXMLReader, "TYPE_ELEMENT", INT2FIX(XML_READER_TYPE_ELEMENT));
  rb_define_const(cXMLReader, "TYPE_ATTRIBUTE", INT2FIX(XML_READER_TYPE_ATTRIBUTE));
  rb_define_const(cXMLReader, "TYPE_TEXT", INT2FIX(XML_READER_TYPE_TEXT));
  rb_define_const(cXMLReader, "TYPE_CDATA", INT2FIX(XML_READER_TYPE_CDATA));
  rb_define_const(cXMLReader, "TYPE_ENTITY_REFERENCE", INT2FIX(XML_READER_TYPE_ENTITY_REFERENCE));
  rb_define_const(cXMLReader, "TYPE_ENTITY", INT2FIX(XML_READER_TYPE_ENTITY));
  rb_define_const(cXMLReader, "TYPE_PROCESSING_INSTRUCTION", INT2FIX(XML_READER_TYPE_PROCESSING_INSTRUCTION));
  rb_define_const(cXMLReader, "TYPE_COMMENT", INT2FIX(XML_READER_TYPE_COMMENT));
  rb_define_const(cXMLReader, "TYPE_DOCUMENT",       INT2FIX(XML_READER_TYPE_DOCUMENT));
  rb_define_const(cXMLReader, "TYPE_DOCUMENT_TYPE", INT2FIX(XML_READER_TYPE_DOCUMENT_TYPE));
  rb_define_const(cXMLReader, "TYPE_DOCUMENT_FRAGMENT", INT2FIX(XML_READER_TYPE_DOCUMENT_FRAGMENT));
  rb_define_const(cXMLReader, "TYPE_NOTATION",       INT2FIX(XML_READER_TYPE_NOTATION));
  rb_define_const(cXMLReader, "TYPE_WHITESPACE", INT2FIX(XML_READER_TYPE_WHITESPACE));
  rb_define_const(cXMLReader, "TYPE_SIGNIFICANT_WHITESPACE", INT2FIX(XML_READER_TYPE_SIGNIFICANT_WHITESPACE));
  rb_define_const(cXMLReader, "TYPE_END_ELEMENT", INT2FIX(XML_READER_TYPE_END_ELEMENT));
  rb_define_const(cXMLReader, "TYPE_END_ENTITY", INT2FIX(XML_READER_TYPE_END_ENTITY));
  rb_define_const(cXMLReader, "TYPE_XML_DECLARATION", INT2FIX(XML_READER_TYPE_XML_DECLARATION));

  /* Read states */
  rb_define_const(cXMLReader, "MODE_INITIAL", INT2FIX(XML_TEXTREADER_MODE_INITIAL));
  rb_define_const(cXMLReader, "MODE_INTERACTIVE", INT2FIX(XML_TEXTREADER_MODE_INTERACTIVE));
  rb_define_const(cXMLReader, "MODE_ERROR", INT2FIX(XML_TEXTREADER_MODE_ERROR));
  rb_define_const(cXMLReader, "MODE_EOF", INT2FIX(XML_TEXTREADER_MODE_EOF));
  rb_define_const(cXMLReader, "MODE_CLOSED",       INT2FIX(XML_TEXTREADER_MODE_CLOSED));
  rb_define_const(cXMLReader, "MODE_READING", INT2FIX(XML_TEXTREADER_MODE_READING));
}
