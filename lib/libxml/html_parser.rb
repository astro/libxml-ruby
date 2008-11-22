module LibXML
  module XML
    class HTMLParser
      def self.file(value)
        parser = self.new
        parser.input.file = value
        parser
      end

      def self.string(value)
        parser = self.new
        parser.input.string = value
        parser
      end

      def self.document(value)
        parser = self.new
        parser.input.document = value
        parser
      end

      def self.io(value)
        parser = self.new
        parser.input.io = value
        parser
      end

      def file
        input.filename
      end

      def file=(value)
        input.file = value
      end

      def string
        input.string
      end

      def string=(value)
        input.string = value
      end

      def document
        input.document
      end

      def document=(value)
        input.document = value
      end

      def io
        input.io
      end

      def io=(value)
        input.io = value
      end
    end
  end
end