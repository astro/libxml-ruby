require 'net/ftp'
require 'rake/clean'
require 'rake/testtask'
require 'rake/rdoctask'
require 'rake/gempackagetask'

CLEAN.include '**/*.o'
CLEAN.include '**/*.so'

CLOBBER.include 'doc/rdoc'
CLOBBER.include '**/*.log'
CLOBBER.include '**/Makefile'
CLOBBER.include '**/extconf.h'

desc "Default task (compile, test and memory test)"
task :default => [:compile, :test, :memory]

desc "Compile extensions."
task :compile do
  sh "task/make"
end

desc "Run unit test suite."
task :test do
  sh "task/test"
end

desc "Run memory test suite."
task :memory do
  sh "task/memory"
end

desc "Generate rdoc documentation."
task :rdoc do
  sh "task/rdoc"
end

desc "Install to local site directory."
task :setup do
  sh "task/setup"
end

__END__

# THIS IS THE OLD RAKEFILE. WE LEAVE IT HERE FOR THE TIME BEING
# SO WE CAN REFERENCE IT TO IMPROVE THE NEW SCRIPTS IF NEEDED.

# Determine the current version of the software
if File.read('ext/xml/libxml.h') =~ /\s*RUBY_LIBXML_VERSION\s*['"](\d.+)['"]/
  CURRENT_VERSION = $1
else
  CURRENT_VERSION = "0.0.0"
end

if ENV['REL']
  PKG_VERSION = ENV['REL']
else
  PKG_VERSION = CURRENT_VERSION
end

task :test_ver do
  puts PKG_VERSION
end

# Make tasks -----------------------------------------------------
MAKECMD = ENV['MAKE_CMD'] || 'make'
MAKEOPTS = ENV['MAKE_OPTS'] || ''

file 'ext/xml/Makefile' => 'ext/xml/extconf.rb' do
  Dir.chdir('ext/xml') do
    ruby 'extconf.rb'
  end
end

def make(target = '')
  Dir.chdir('ext/xml') do
    pid = fork { exec "#{MAKECMD} #{MAKEOPTS} #{target}" }
    Process.waitpid pid
  end
  $?.exitstatus
end

# Let make handle dependencies between c/o/so - we'll just run it. 
file 'ext/xml/libxml.so' => 'ext/xml/Makefile' do
  m = make
  fail "Make failed (status #{m})" unless m == 0
end

desc "Compile the shared object"
task :compile => 'ext/xml/libxml.so'

desc "Install to your site_ruby directory"
task :install => :compile do
  m = make 'install' 
  fail "Make install failed (status #{m})" unless m == 0
end

# Test Tasks ---------------------------------------------------------
task :ta => :alltests
task :tu => :unittests
task :test => :unittests
task :tm => :memtests

Rake::TestTask.new(:alltests) do |t|
  t.test_files = FileList[
    'tests/runner.rb',
    'tests/contrib/*.rb',
  ]
  t.verbose = true
end
                    
Rake::TestTask.new(:unittests) do |t|
  t.test_files = FileList['tests/runner.rb']
  t.verbose = false
end

Rake::TestTask.new(:memtests) do |t|
  t.test_files = FileList[
                          'tests/runner_ets.rb'
                         ]
  t.verbose = true
end
                          
#Rake::TestTask.new(:funtests) do |t|
  #  t.test_files = FileList['test/fun*.rb']
  #t.warning = true
  #t.warning = true
#end

task :unittests => :compile
task :alltests => :compile
task :memtests => :compile

# RDoc Tasks ---------------------------------------------------------
desc "Create the RDOC documentation tree"
rd = Rake::RDocTask.new(:doc) do |rdoc|
  rdoc.rdoc_dir = 'html'
  rdoc.title    = "Libxml-Ruby API"
  rdoc.options << '--main' << 'README'
  rdoc.rdoc_files.include('README', 'LICENSE', 'TODO')
  rdoc.rdoc_files.include('ext/xml/ruby_xml*.c', 'ext/xml/libxml.rb')
  rdoc.rdoc_files.include('*.rdoc')
end

desc "Publish the RDoc documentation to project web site"
task :pubdoc => [ :doc ] do
  unless ENV['RUBYFORGE_ACCT']
    raise "Need to set RUBYFORGE_ACCT to your rubyforge.org user name (e.g. 'fred')"
  end
  require 'rake/contrib/sshpublisher'
  Rake::SshDirPublisher.new(
    "#{ENV['RUBYFORGE_ACCT']}@rubyforge.org",
    "/var/www/gforge-projects/libxml/doc",
    "html"
  ).upload
end

# Packaging / Version number tasks -----------------------------------
# Used during release packaging if a REL is supplied
task :update_version do
  unless PKG_VERSION == CURRENT_VERSION
    pkg_vernum = PKG_VERSION.tr('.','').sub(/^0*/,'')
    pkg_vernum << '0' until pkg_vernum.length > 2
    
    File.open('ext/xml/libxml.h.new','w+') do |f|      
    maj, min, mic, patch = /(\d+)\.(\d+)(?:\.(\d+))?(?:\.(\d+))?/.match(PKG_VERSION).captures
      f << File.read('ext/xml/libxml.h').
           gsub(/RUBY_LIBXML_VERSION\s+"(\d.+)"/) { "RUBY_LIBXML_VERSION  \"#{PKG_VERSION}\"" }.
           gsub(/RUBY_LIBXML_VERNUM\s+\d+/) { "RUBY_LIBXML_VERNUM   #{pkg_vernum}" }.
           gsub(/RUBY_LIBXML_VER_MAJ\s+\d+/) { "RUBY_LIBXML_VER_MAJ   #{maj}" }.
           gsub(/RUBY_LIBXML_VER_MIN\s+\d+/) { "RUBY_LIBXML_VER_MIN   #{min}" }.
           gsub(/RUBY_LIBXML_VER_MIC\s+\d+/) { "RUBY_LIBXML_VER_MIC   #{mic || 0}" }.
           gsub(/RUBY_LIBXML_VER_PATCH\s+\d+/) { "RUBY_LIBXML_VER_PATCH #{patch || 0}" }           
    end
    mv('ext/xml/libxml.h.new', 'ext/xml/libxml.h')     
  end
end

PKG_FILES = FileList[
  'ext/xml/*.rb',
  'ext/xml/extconf.rb',
  '[A-Z]*',
  'ext/xml/*.c',
  'ext/xml/*.inc', 
  'ext/xml/ruby_xml*.h',
  'ext/xml/libxml.h',
  'tests/**/*',
]

if ! defined?(Gem)
  warn "Package Target requires RubyGEMs"
else
  spec = Gem::Specification.new do |s|
    
    #### Basic information.

    s.name = 'libxml-ruby'
    s.version = PKG_VERSION
    s.summary = "LibXML2 bindings for Ruby"
    s.description = <<-EOF
      C-language bindings for Gnome LibXML2 and Ruby. Supports
      high-speed, feature rich XML processing in Ruby apps.
    EOF
    s.extensions = 'ext/xml/extconf.rb'    

    #### Which files are to be included in this gem? 

    s.files = PKG_FILES.to_a

    #### Load-time details
    s.require_path = 'lib'
    
    #### Documentation and testing.
    s.has_rdoc = true
    s.extra_rdoc_files = rd.rdoc_files.to_a
    s.rdoc_options <<
      '--title' <<  'Libxml-Ruby API' <<
      '--main' << 'README'

    s.test_files = Dir.glob('tests/*runner.rb')
    
    #### Author and project details.

    s.author = "Sean Chittenden"
    s.email = "libxml-devel@rubyforge.org"
    s.homepage = "http://libxml.rubyforge.org"
    s.rubyforge_project = "libxml"
  end
  
  # Quick fix for Ruby 1.8.3 / YAML bug
  if (RUBY_VERSION == '1.8.3')
    def spec.to_yaml
      out = super
      out = '--- ' + out unless out =~ /^---/
      out
    end  
  end

  package_task = Rake::GemPackageTask.new(spec) do |pkg|
    pkg.need_zip = true
    pkg.need_tar_gz = true
    pkg.package_dir = 'pkg'    
  end
      
  desc "Build a full release"
  task :release => [:clobber, :update_version, :compile, :test, :package]
end
