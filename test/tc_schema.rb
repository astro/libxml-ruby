require 'xml'
require 'test/unit'

class TestSchema < Test::Unit::TestCase
  def setup
    xp = XML::Parser.new
    @doc = XML::Document.file(File.join(File.dirname(__FILE__), 'model/shiporder.xml'))
  end
  
  def teardown
    @doc = nil
  end
  
  def schema
    document = XML::Document.file(File.join(File.dirname(__FILE__), 'model/shiporder.xsd'))
    schema = XML::Schema.document(document)
  end

  def test_from_doc
    assert_instance_of(XML::Schema, schema)
  end
  
  def test_valid
    assert(@doc.validate_schema(schema))
  end
  
  def test_invalid
    new_node = XML::Node.new('invalid', 'this will mess up validation')
    @doc.root.child_add(new_node)

    error = assert_raise(XML::Error) do
      @doc.validate_schema(schema)
    end

    assert_not_nil(error)
    assert_kind_of(XML::Error, error)
    assert_equal("Error: Element 'invalid': This element is not expected. Expected is ( item ). at C:/Development/src/libxml-ruby/test/model/shiporder.xml:0.",
                 error.message)
    assert_equal(XML::Error::XML_FROM_SCHEMASV, error.domain)
    assert_equal(XML::Error::XML_SCHEMAV_ELEMENT_CONTENT, error.code)
    assert_equal(XML::Error::XML_ERR_ERROR, error.level)
    assert_equal("C:/Development/src/libxml-ruby/test/model/shiporder.xml", error.file)
    assert_nil(error.line)
    assert_nil(error.str1)
    assert_nil(error.str2)
    assert_nil(error.str3)
    assert_equal(0, error.int1)
    assert_equal(0, error.int2)
    assert_not_nil(error.node)
    assert_equal('invalid', error.node.name)
  end
end
