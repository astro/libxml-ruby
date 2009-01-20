require 'xml'
require 'test/unit'
require 'pp'

class DocTypeCallback
  include XML::SaxParser::Callbacks
  def on_start_element(element, attributes)
  end
end

class TestCaseCallbacks
  include XML::SaxParser::Callbacks

  attr_accessor :result

  def initialize
    @result = Array.new
  end

  def on_cdata_block(cdata)
    @result << "cdata: #{cdata}"
  end

  def on_characters(chars)
    @result << "characters: #{chars}"
  end

  def on_comment(text)
    @result << "comment: #{text}"
  end

  def on_end_document
    @result << "end_document"
  end

  def on_end_element(name)
    @result << "end_element: #{name}"
  end

  def on_end_element_ns(name, prefix, uri)
    @result << "end_element_ns #{name}, prefix: #{prefix}, uri: #{uri}"
  end

  # Called for parser errors.
  def on_error(error)
    @result << "error: #{error}"
  end

  def on_processing_instruction(target, data)
    @result << "pi: #{target} #{data}"
  end

  def on_start_document
    @result << "startdoc"
  end

  def on_start_element(name, attributes)
    attributes ||= Hash.new
    @result << "start_element: #{name}, attr: #{attributes.inspect}"
  end

  def on_start_element_ns(name, attributes, prefix, uri, namespaces)
    attributes ||= Hash.new
    namespaces ||= Hash.new
    @result << "start_element_ns: #{name}, attr: #{attributes.inspect}, prefix: #{prefix}, uri: #{uri}, ns: #{namespaces.inspect}"
  end
end

class TestSaxParser < Test::Unit::TestCase
  def saxtest_file
    File.join(File.dirname(__FILE__), 'model/atom.xml')
  end

  def verify(parser)
    result = parser.callbacks.result

    i = -1
    assert_equal("startdoc", result[i+=1])
    assert_equal("pi: xml-stylesheet type=\"text/xsl\" href=\"my_stylesheet.xsl\"", result[i+=1])
    assert_equal("start_element: feed, attr: {nil=>\"http://www.w3.org/2005/Atom\"}", result[i+=1])
    assert_equal("start_element_ns: feed, attr: {nil=>\"http://www.w3.org/2005/Atom\"}, prefix: , uri: http://www.w3.org/2005/Atom, ns: {}", result[i+=1])
    assert_equal("characters: \n  ", result[i+=1])
    assert_equal("comment:  Not a valid atom entry ", result[i+=1])
    assert_equal("characters: \n  ", result[i+=1])
    assert_equal("start_element: entry, attr: {}", result[i+=1])
    assert_equal("start_element_ns: entry, attr: {}, prefix: , uri: http://www.w3.org/2005/Atom, ns: {}", result[i+=1])
    assert_equal("characters: \n    ", result[i+=1])
    assert_equal("start_element: title, attr: {\"type\"=>\"html\"}", result[i+=1])
    assert_equal("start_element_ns: title, attr: {\"type\"=>\"html\"}, prefix: , uri: http://www.w3.org/2005/Atom, ns: {}", result[i+=1])
    assert_equal("cdata: <<strong>>", result[i+=1])
    assert_equal("end_element: title", result[i+=1])
    assert_equal("end_element_ns title, prefix: , uri: http://www.w3.org/2005/Atom", result[i+=1])
    assert_equal("characters: \n    ", result[i+=1])
    assert_equal("start_element: content, attr: {\"type\"=>\"xhtml\"}", result[i+=1])
    assert_equal("start_element_ns: content, attr: {\"type\"=>\"xhtml\"}, prefix: , uri: http://www.w3.org/2005/Atom, ns: {}", result[i+=1])
    assert_equal("characters: \n      ", result[i+=1])
    assert_equal("start_element: xhtml:div, attr: {\"xhtml\"=>\"http://www.w3.org/1999/xhtml\"}", result[i+=1])
    assert_equal("start_element_ns: div, attr: {\"xhtml\"=>\"http://www.w3.org/1999/xhtml\"}, prefix: xhtml, uri: http://www.w3.org/1999/xhtml, ns: {}", result[i+=1])
    assert_equal("characters: \n        ", result[i+=1])
    assert_equal("start_element: xhtml:p, attr: {}", result[i+=1])
    assert_equal("start_element_ns: p, attr: {}, prefix: xhtml, uri: http://www.w3.org/1999/xhtml, ns: {}", result[i+=1])
    assert_equal("characters: hi there", result[i+=1])
    assert_equal("end_element: xhtml:p", result[i+=1])
    assert_equal("end_element_ns p, prefix: xhtml, uri: http://www.w3.org/1999/xhtml", result[i+=1])
    assert_equal("characters: \n      ", result[i+=1])
    assert_equal("end_element: xhtml:div", result[i+=1])
    assert_equal("end_element_ns div, prefix: xhtml, uri: http://www.w3.org/1999/xhtml", result[i+=1])
    assert_equal("characters: \n    ", result[i+=1])
    assert_equal("end_element: content", result[i+=1])
    assert_equal("end_element_ns content, prefix: , uri: http://www.w3.org/2005/Atom", result[i+=1])
    assert_equal("characters: \n  ", result[i+=1])
    assert_equal("end_element: entry", result[i+=1])
    assert_equal("end_element_ns entry, prefix: , uri: http://www.w3.org/2005/Atom", result[i+=1])
    assert_equal("characters: \n", result[i+=1])
    assert_equal("end_element: feed", result[i+=1])
    assert_equal("end_element_ns feed, prefix: , uri: http://www.w3.org/2005/Atom", result[i+=1])
    assert_equal("end_document", result[i+=1])
  end

  def test_string_no_callbacks
    xml = File.read(saxtest_file)
    parser = XML::SaxParser.string(xml)
    assert_equal true, parser.parse
  end

  def test_file_no_callbacks
    parser = XML::SaxParser.file(saxtest_file)
    assert_equal true, parser.parse
  end

  def test_string
    xml = File.read(saxtest_file)
    parser = XML::SaxParser.string(xml)
    parser.callbacks = TestCaseCallbacks.new
    parser.parse
    verify(parser)
  end

  def test_file
    parser = XML::SaxParser.file(saxtest_file)
    parser.callbacks = TestCaseCallbacks.new
    parser.parse
    verify(parser)
  end

  def test_io
    File.open(saxtest_file) do |file|
      parser = XML::SaxParser.io(file)
      parser.callbacks = TestCaseCallbacks.new
      parser.parse
      verify(parser)
    end
  end

  def test_string_io
    xml = File.read(saxtest_file)
    io = StringIO.new(xml)
    parser = XML::SaxParser.io(io)
    
    parser.callbacks = TestCaseCallbacks.new
    parser.parse
    verify(parser)
  end

  def test_doctype
    xml = <<-EOS
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Results SYSTEM "results.dtd">
<Results>
  <a>a1</a>
</Results>
EOS
    parser = XML::SaxParser.string(xml)
    parser.callbacks = DocTypeCallback.new
    doc = parser.parse
    assert_not_nil(doc)
  end


  def test_parse_warning
    # Two xml PIs is a warning
    xml = <<-EOS
<?xml version="1.0" encoding="utf-8"?>
<?xml-invalid?>
<Test/>
EOS

    parser = XML::SaxParser.string(xml)
    parser.callbacks = TestCaseCallbacks.new

    parser.parse

    # Check callbacks
    result = parser.callbacks.result
    i = -1
    assert_equal("startdoc", result[i+=1])
    assert_equal("error: Warning: xmlParsePITarget: invalid name prefix 'xml' at :2.", result[i+=1])
    assert_equal("pi: xml-invalid ", result[i+=1])
    assert_equal("start_element: Test, attr: {}", result[i+=1])
    assert_equal("start_element_ns: Test, attr: {}, prefix: , uri: , ns: {}", result[i+=1])
    assert_equal("end_element: Test", result[i+=1])
    assert_equal("end_element_ns Test, prefix: , uri: ", result[i+=1])
    assert_equal("end_document", result[i+=1])
  end

  def test_parse_error
    xml = <<-EOS
      <Results>
    EOS
    parser = XML::SaxParser.string(xml)
    parser.callbacks = TestCaseCallbacks.new

    error = assert_raise(XML::Error) do
      doc = parser.parse
    end

    # Check callbacks
    result = parser.callbacks.result

    i = -1
    assert_equal("startdoc", result[i+=1])
    assert_equal("start_element: Results, attr: {}", result[i+=1])
    assert_equal("start_element_ns: Results, attr: {}, prefix: , uri: , ns: {}", result[i+=1])
    assert_equal("characters: \n", result[i+=1])
    assert_equal("error: Fatal error: Premature end of data in tag Results line 1 at :2.", result[i+=1])
    assert_equal("end_document", result[i+=1])

    assert_not_nil(error)
    assert_kind_of(XML::Error, error)
    assert_equal("Fatal error: Premature end of data in tag Results line 1 at :2.", error.message)
    assert_equal(XML::Error::PARSER, error.domain)
    assert_equal(XML::Error::TAG_NOT_FINISHED, error.code)
    assert_equal(XML::Error::FATAL, error.level)
    assert_nil(error.file)
    assert_equal(2, error.line)
    assert_equal('Results', error.str1)
    assert_nil(error.str2)
    assert_nil(error.str3)
    assert_equal(1, error.int1)
    assert_equal(1, error.int2)
    assert_nil(error.node)
  end
end