/* $Id$ */

#include "ruby_libxml.h"

VALUE cXMLState;
VALUE LIBXML_STATE = Qnil;

static int dummy = 0;

void
ruby_xml_state_free(int dummy) {
  xmlCleanupParser();
  LIBXML_STATE = Qnil;
}

VALUE
ruby_xml_state_alloc(VALUE klass) {
  #ifdef DEBUG
  fprintf(stderr, "Allocating state");
  #endif
  
  xmlInitParser();
  
  return Data_Wrap_Struct(cXMLState,
			  NULL, ruby_xml_state_free,
			  &dummy);
}

// Rdoc needs to know 
#ifdef RDOC_NEVER_DEFINED
  mLibXML = rb_define_module("LibXML");
#endif

void
ruby_init_state(void) {	
  VALUE rb_mSingleton;
  cXMLState = rb_define_class_under(mXML, "State", rb_cObject);
  
  /* Mixin singleton so only one xml state object can be created. */
  rb_require("singleton");
  rb_mSingleton = rb_const_get(rb_cObject, rb_intern("Singleton"));
  rb_include_module(cXMLState, rb_mSingleton);

  rb_define_alloc_func(cXMLState, ruby_xml_state_alloc);

  /* Create one instance of the state object that is used
     to initalize and cleanup libxml. Then register it with
     the garbage collector so its not freed until the process
     exists.*/
  LIBXML_STATE = rb_class_new_instance(0, NULL, cXMLState);
  rb_global_variable(&LIBXML_STATE);  
}
