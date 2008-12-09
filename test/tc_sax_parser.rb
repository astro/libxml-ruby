require 'xml'
require 'test/unit'

class DocTypeCallback
  include XML::SaxParser::Callbacks
  def on_start_element(element, attributes)
  end
end

class TestCaseCallbacks
  include XML::SaxParser::Callbacks

  attr_accessor :test

  def initialize
    @test = Hash.new { |h,k| h[k] = [] }
    @i = 0
  end

  def on_start_document
    @test[:startdoc] << @i+=1
  end

  def on_start_element(name, attr_hash)
    @test[:startel] << [@i+=1,name,attr_hash]
  end

  def on_characters(chars)
    @test[:chars] << [@i+=1,chars]
  end

  def on_comment(msg)
    @test[:comment] << [@i+=1,msg]
  end

  def on_processing_instruction(target, data)
    @test[:pinstr] << [@i+=1, target, data]
  end

  def on_cdata_block(cdata)
    @test[:cdata] << [@i+=1,cdata]
  end

  def on_end_element(name)
    @test[:endel] << [@i+=1,name]
  end

  def on_end_document
    @test[:enddoc] << @i+=1
  end
end

class TestSaxParser < Test::Unit::TestCase
  def setup
    @xp = XML::SaxParser.new
  end

  def teardown
    @xp = nil
  end

  def saxtest_file
    File.join(File.dirname(__FILE__), 'model/saxtest.xml')
  end

  def verify
    assert_equal [1], @xp.callbacks.test[:startdoc]
    assert_equal [[2,'test',{'uga'=>'booga','foo'=>'bar'}],[3,'fixnum',{}],[6,'fixnum',{}]],
                 @xp.callbacks.test[:startel]
    assert_equal [[4,'one'],[7,'two'],[9,"\n  "],[11,"\n  "],[13,"\n  "],[15,"\n"]],
                 @xp.callbacks.test[:chars]
    assert_equal [[10, ' msg ']], @xp.callbacks.test[:comment]
    assert_equal [[12, 'custom', 'foo="bar"']], @xp.callbacks.test[:pinstr]
    assert_equal [[14, 'here it goes']], @xp.callbacks.test[:cdata]
    assert_equal [[5,'fixnum'],[8,'fixnum'],[16,'test']], @xp.callbacks.test[:endel]
    assert_equal [17], @xp.callbacks.test[:enddoc]
  end
  
#  def test_string_no_callbacks
#    @xp.string = File.read(saxtest_file)
#    assert_equal true, @xp.parse
#  end
#
#  def test_file_no_callbacks
#    @xp.file = File.join(saxtest_file)
#    assert_equal true, @xp.parse
#  end
#
#  def test_string
#    @xp.callbacks = TestCaseCallbacks.new
#    @xp.string = File.read(saxtest_file)
#    @xp.parse
#    verify
#  end
#
#  def test_file
#    @xp.callbacks = TestCaseCallbacks.new
#    @xp.file = saxtest_file
#    @xp.parse
#    verify
#  end
#
#  def test_io
#    File.open(saxtest_file) do |file|
#      @xp.callbacks = TestCaseCallbacks.new
#      @xp.io = file
#      @xp.parse
#      verify
#    end
#  end
#
#  def test_string_io
#    data = File.read(saxtest_file)
#
#    @xp.callbacks = TestCaseCallbacks.new
#    @xp.io = StringIO.new(data)
#    @xp.parse
#    verify
#  end
#
#  def test_doctype
#    @xp.callbacks = DocTypeCallback.new
#    @xp.string = <<-EOS
#<?xml version="1.0" encoding="UTF-8"?>
#<!DOCTYPE Results SYSTEM "results.dtd">
#<Results>
#  <a>a1</a>
#</Results>
#EOS
#    doc = @xp.parse
#    assert_not_nil(doc)
#  end
#
  def test_parse_error
    @xp.callbacks = TestCaseCallbacks.new
    @xp.string = <<-EOS
      <Results>
        <a>a1
      </Results>
    EOS

    error = assert_raise(XML::Error) do
      doc = @xp.parse
    end

    assert_not_nil(error)
    assert_kind_of(XML::Error, error)
    assert_equal("Fatal error: Premature end of data in tag Results line 1 at :4.", error.message)
    assert_equal(XML::Error::PARSER, error.domain)
    assert_equal(XML::Error::TAG_NOT_FINISHED, error.code)
    assert_equal(XML::Error::FATAL, error.level)
    assert_nil(error.file)
    assert_equal(4, error.line)
    assert_equal('Results', error.str1)
    assert_nil(error.str2)
    assert_nil(error.str3)
    assert_equal(1, error.int1)
    assert_equal(1, error.int2)
    assert_nil(error.node)
  end
end