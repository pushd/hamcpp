# -*- ruby -*-

require "rubygems"
require "rake"
require "rake/extensiontask"
require "hoe"

# Hoe.plugin :compiler
# Hoe.plugin :gem_prelude_sucks
# Hoe.plugin :inline
# Hoe.plugin :minitest
# Hoe.plugin :racc
# Hoe.plugin :rcov
# Hoe.plugin :rdoc

Hoe.spec "hamcpp" do
  # HEY! If you fill these out in ~/.hoe_template/default/Rakefile.erb then
  # you'll never have to touch them again!
  # (delete this comment too, of course)

  developer("Alek Kolcz", "alek@pushd.com")

  # license "MIT" # this should match the license in the README
  self.extra_dev_deps << ['rake-compiler', '>= 1.0.1']
  self.extra_dev_deps << ['rice', '>= 2.1.2']
  self.spec_extras = { :extensions => ["ext/hamcpp/extconf.rb"] }
  spec = Gem::Specification.load('hamcpp.gemspec')

  Rake::ExtensionTask.new('hamcpp', spec) do |ext|
    ext.lib_dir = File.join('lib', 'hamcpp')
  end
end

Rake::Task[:test].prerequisites << :compile

# vim: syntax=ruby
