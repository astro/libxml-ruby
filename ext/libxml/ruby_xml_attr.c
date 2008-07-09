/* $Id$ */

/* Please see the LICENSE file for copyright and distribution information */

/*
* Document-class: XML::Attr
*
* Provides access to an single element attribute.  Accessed by
* calling XML::Node#attributes method.
*
* Basic Usage:
*  require 'libxml'
*
*  doc = XML::Document.new(<some_file>)
*  attribute = doc.root.attributes.get_attribute_ns('http://www.w3.org/1999/xlink', 'href')
*  attribute.name == 'href'
*  attribute.value == 'http://www.mydocument.com'
*  attribute.remove!
*/

#include "ruby_libxml.h"
#include "ruby_xml_attr.h"

VALUE cXMLAttr;

void ruby_xml_attr_free(xmlAttrPtr xattr) {
  if (!xattr) return;

  if (xattr != NULL ) {
    xattr->_private=NULL;
    if (xattr->parent == NULL && xattr->doc == NULL ) {
#ifdef NODE_DEBUG
      fprintf(stderr,"ruby_xfree rxn=0x%x xn=0x%x o=0x%x\n",(long)rxn,(long)rxn->node,(long)rxn->node->_private);
#endif
      xmlFreeProp(xattr);
    }

    xattr=NULL;
  }
}

void
ruby_xml_attr_mark(xmlAttrPtr xattr) {
  if (xattr == NULL) return;

  if (xattr->_private == NULL ) {
    rb_warning("XmlAttr is not bound! (%s:%d)",
	       __FILE__,__LINE__);
    return;
  }

  ruby_xml_node_mark_common((xmlNodePtr)xattr);
}

VALUE
ruby_xml_attr_wrap(xmlAttrPtr xattr)
{
  VALUE result;
  // This node is already wrapped
  if (xattr->_private != NULL)
    return (VALUE)xattr->_private;

  result = Data_Wrap_Struct(cXMLAttr,
                            ruby_xml_attr_mark, ruby_xml_attr_free,
                            xattr);

  xattr->_private=(void*)result;
#ifdef NODE_DEBUG
  fprintf(stderr,"wrap rxn=0x%x xn=0x%x o=0x%x\n",(long)rxn,(long)xnode,(long)obj);
#endif
  return result;
}


VALUE
ruby_xml_attr_alloc(VALUE klass)
{
  return Data_Wrap_Struct(cXMLAttr,
                          ruby_xml_attr_mark, ruby_xml_attr_free,
                          NULL);
}

/*
 * call-seq:
 *    attr.initialize(node, "name", "value")
 * 
 * Creates a new attribute for the node.
 *
 * node: The XML::Node that will contain the attribute
 * name: The name of the attribute
 * value: The value of the attribute
 *
 *  attr = XML::Attr.new(doc.root, 'name', 'libxml')
 */
VALUE
ruby_xml_attr_initialize(int argc, VALUE *argv, VALUE self) {
  VALUE node = argv[0];
  VALUE name = argv[1];
  VALUE value = argv[2];
  VALUE ns = (argc == 4 ? argv[3] : Qnil);

  xmlNodePtr xnode;
  xmlAttrPtr xattr;
  
  if ( argc < 3 || argc > 4 )
    rb_raise(rb_eArgError, "wrong number of arguments (3 or 4)");

  Check_Type(name, T_STRING);
  Check_Type(value, T_STRING);
  
  Data_Get_Struct(node, xmlNode, xnode);
  
  if NIL_P(ns)
  {
    xattr = xmlNewProp(xnode, (xmlChar*)StringValuePtr(name), (xmlChar*)StringValuePtr(value));
  }
  else
  {
    xmlNsPtr xns;
    Data_Get_Struct(ns, xmlNs, xns);
    xattr = xmlNewNsProp(xnode, xns, (xmlChar*)StringValuePtr(name), (xmlChar*)StringValuePtr(value));
  }
  
  xattr->_private = (void *)self;
  DATA_PTR(self) = xattr;
  return self;
}

/*
 * call-seq:
 *    attr.child -> node
 * 
 * Obtain this attribute's child attribute(s).
 */
VALUE
ruby_xml_attr_child_get(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->children == NULL)
    return(Qnil);
  else
    return(ruby_xml_node2_wrap(cXMLNode, (xmlNodePtr)xattr->children));
}


/*
 * call-seq:
 *    attr.child? -> (true|false)
 * 
 * Determine whether this attribute has child attributes.
 */
VALUE
ruby_xml_attr_child_q(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->children == NULL)
    return(Qfalse);
  else
    return(Qtrue);
}


/*
 * call-seq:
 *    attr.doc -> XML::Document
 * 
 * Returns this attribute's document.
 *
 *  doc.root.attributes.get_attribute('name').doc == doc
 */
VALUE
ruby_xml_attr_doc_get(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->doc == NULL)
    return(Qnil);
  else
    return(ruby_xml_document_wrap(xattr->doc));
}

/*
 * call-seq:
 *    attr.doc? -> (true|false)
 * 
 * Determine whether this attribute is associated with an
 * XML::Document.
 */
VALUE
ruby_xml_attr_doc_q(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->doc == NULL)
    return(Qfalse);
  else
    return(Qtrue);
}


/*
 * call-seq:
 *    attr.last -> node
 * 
 * Obtain the last attribute.
 */
VALUE
ruby_xml_attr_last_get(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->last == NULL)
    return(Qnil);
  else
    return(ruby_xml_node2_wrap(cXMLNode, xattr->last));
}


/*
 * call-seq:
 *    attr.last? -> (true|false)
 * 
 * Determine whether this is the last attribute.
 */
VALUE
ruby_xml_attr_last_q(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->last == NULL)
    return(Qfalse);
  else
    return(Qtrue);
}

/*
 * call-seq:
 *    attr.name -> "name"
 * 
 * Obtain this attribute's name.
 */
VALUE
ruby_xml_attr_name_get(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);

  if (xattr->name == NULL)
    return(Qnil);
  else
    return(rb_str_new2((const char*)xattr->name));
}

/*
 * call-seq:
 *    attr.next -> node
 * 
 * Obtain the next attribute.
 */
VALUE
ruby_xml_attr_next_get(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->next == NULL)
    return(Qnil);
  else
    return(ruby_xml_attr_wrap(xattr->next));
}


/*
 * call-seq:
 *    attr.next? -> (true|false)
 * 
 * Determine whether there is a next attribute.
 */
VALUE
ruby_xml_attr_next_q(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->next == NULL)
    return(Qfalse);
  else
    return(Qtrue);
}


/*
 * call-seq:
 *    attr.type_name -> "attribute"
 * 
 * Obtain this attribute node's type name.
 */
VALUE
ruby_xml_attr_node_type_name(VALUE self) {
  return(rb_str_new2("attribute"));
}


/*
 * call-seq:
 *    attr.ns -> namespace
 * 
 * Obtain this attribute's associated XML::NS, if any.
 */
VALUE
ruby_xml_attr_ns_get(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->ns == NULL)
    return(Qnil);
  else
    return(ruby_xml_ns_wrap(xattr->ns));
}


/*
 * call-seq:
 *    attr.ns? -> (true|false)
 * 
 * Determine whether this attribute has an associated 
 * namespace.
 */
VALUE
ruby_xml_attr_ns_q(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->ns == NULL)
    return(Qfalse);
  else
    return(Qtrue);
}


/*
 * call-seq:
 *    attr.parent -> node
 * 
 * Obtain this attribute node's parent.
 */
VALUE
ruby_xml_attr_parent_get(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->parent == NULL)
    return(Qnil);
  else
    return(ruby_xml_node2_wrap(cXMLNode, xattr->parent));
}


/*
 * call-seq:
 *    attr.parent? -> (true|false)
 * 
 * Determine whether this attribute has a parent.
 */
VALUE
ruby_xml_attr_parent_q(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->parent == NULL)
    return(Qfalse);
  else
    return(Qtrue);
}


/*
 * call-seq:
 *    attr.prev -> node
 * 
 * Obtain the previous attribute.
 */
VALUE
ruby_xml_attr_prev_get(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->prev == NULL)
    return(Qnil);
  else
    return(ruby_xml_attr_wrap(xattr->prev));
}


/*
 * call-seq:
 *    attr.prev? -> (true|false)
 * 
 * Determine whether there is a previous attribute.
 */
VALUE
ruby_xml_attr_prev_q(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  if (xattr->prev == NULL)
    return(Qfalse);
  else
    return(Qtrue);
}


/*
 * call-seq:
 *     node.remove! -> nil
 * 
 * Removes this attribute from it's parent.
 */
VALUE
ruby_xml_attr_remove_ex(VALUE self) {
  xmlAttrPtr xattr;
  Data_Get_Struct(self, xmlAttr, xattr);
  
  if (xattr->_private == NULL)
    xmlRemoveProp(xattr);
  else
    xmlUnlinkNode((xmlNodePtr)xattr);

  return(Qnil);
}

/*
 * call-seq:
 *    attr.value -> "value"
 * 
 * Obtain the value of this attribute.
 */
VALUE
ruby_xml_attr_value_get(VALUE self) {
  xmlAttrPtr xattr;
  xmlChar *value;
  VALUE result = Qnil;
  
  Data_Get_Struct(self, xmlAttr, xattr);
  if (ruby_xml_attr_parent_q(self) == Qtrue) {
    value = xmlGetProp(xattr->parent, xattr->name);
    if (value != NULL)
    {
      result = rb_str_new2((const char*)value);
      xmlFree(value);
    }
  }
  return(result);
}


/*
 * call-seq:
 *    attr.value = "value"
 * 
 * Sets the value of this attribute.
 */
VALUE
ruby_xml_attr_value_set(VALUE self, VALUE val) {
  xmlAttrPtr xattr;
  
  Check_Type(val, T_STRING);
  Data_Get_Struct(self, xmlAttr, xattr);

  if (xattr->ns)
    xmlSetNsProp(xattr->parent, xattr->ns, xattr->name, (xmlChar*)StringValuePtr(val));
  else
    xmlSetProp(xattr->parent, xattr->name, (xmlChar*)StringValuePtr(val));
    
  return(self);
}


// Rdoc needs to know 
#ifdef RDOC_NEVER_DEFINED
  mXML = rb_define_module("XML");
#endif

void
ruby_init_xml_attr(void) {
  cXMLAttr = rb_define_class_under(mXML, "Attr", rb_cObject);
  rb_define_alloc_func(cXMLAttr, ruby_xml_attr_alloc);
  rb_define_method(cXMLAttr, "initialize", ruby_xml_attr_initialize, -1);
  rb_define_method(cXMLAttr, "child", ruby_xml_attr_child_get, 0);
  rb_define_method(cXMLAttr, "child?", ruby_xml_attr_child_q, 0);
  rb_define_method(cXMLAttr, "doc", ruby_xml_attr_doc_get, 0);
  rb_define_method(cXMLAttr, "doc?", ruby_xml_attr_doc_q, 0);
  rb_define_method(cXMLAttr, "last", ruby_xml_attr_last_get, 0);
  rb_define_method(cXMLAttr, "last?", ruby_xml_attr_last_q, 0);
  rb_define_method(cXMLAttr, "name", ruby_xml_attr_name_get, 0);
  rb_define_method(cXMLAttr, "next", ruby_xml_attr_next_get, 0);
  rb_define_method(cXMLAttr, "next?", ruby_xml_attr_next_q, 0);
  rb_define_method(cXMLAttr, "node_type_name", ruby_xml_attr_node_type_name, 0);
  rb_define_method(cXMLAttr, "ns", ruby_xml_attr_ns_get, 0);
  rb_define_method(cXMLAttr, "ns?", ruby_xml_attr_ns_q, 0);
  rb_define_method(cXMLAttr, "parent", ruby_xml_attr_parent_get, 0);
  rb_define_method(cXMLAttr, "parent?", ruby_xml_attr_parent_q, 0);
  rb_define_method(cXMLAttr, "prev", ruby_xml_attr_prev_get, 0);
  rb_define_method(cXMLAttr, "prev?", ruby_xml_attr_prev_q, 0);
  rb_define_method(cXMLAttr, "remove!", ruby_xml_attr_remove_ex, 0);
  rb_define_method(cXMLAttr, "value", ruby_xml_attr_value_get, 0);
  rb_define_method(cXMLAttr, "value=", ruby_xml_attr_value_set, 1);
}
