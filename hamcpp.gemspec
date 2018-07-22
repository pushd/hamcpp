Gem::Specification.new do |s|
  s.name    = "hamcpp"
  s.version = "1.0.0"
  s.summary = "C++ Optimized Hamming Distance for Ruby"
  s.author  = "Alek Kolcz"

  s.files = Dir.glob("ext/**/*.{cpp,h,rb}") +
            Dir.glob("lib/**/*.rb")

  s.extensions << "ext/hamcpp/extconf.rb"
  s.add_development_dependency 'rake-compiler', '~> 1.0', '>= 1.0.1'
  s.add_development_dependency 'rice', '~> 2.1', '>= 2.1.2'
end
