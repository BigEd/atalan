begin
   require 'Win32/Console/ANSI'
rescue LoadError
   raise 'You must gem install win32console to use color on Windows'
end

def colorize(text, color_code)
  "\e[#{color_code}m#{text}\e[0m"
end

def red(text); colorize(text, "31"); end
def green(text); colorize(text, "32"); end


$tests_to_run = ["if_eq", "if_le_e"]
$test_results = []
$bin_path = "atalan/bin"

def compile(test, platform)
	log = `\"#{$bin_path}/atalan.exe\" -v0 -p #{platform} tests/#{test}.atl 2>&1`
	result=$?.success?
	return result
end

def test(test, platform)

	if compile(test, platform)
		
		if platform == "windows"
			log =  `tests/#{test}.exe 2>&1`
			result=$?.success?
			return result 
				
		elsif platform == "con6502"
			ext = "c65"
		elsif platform == "conZ80"
			ext = "z80"
		end
			
		log =  `\"#{$bin_path}/#{platform}\" tests/#{test}.#{ext} 2>&1`
		result=$?.success?
		return result 
		
	end
	return false
end

# Append tests not mentioned

Dir.glob('tests/*.atl') do |atl_file|
	name = atl_file.match(/tests\/(.+)\.atl/)[1]
	$tests_to_run.push name unless $tests_to_run.include?(name) 
end

$c6502_OK = 0
$Z80_OK = 0
$WIN_OK = 0
$test_count = 0
$test_ok = 0

$Z80_do = false
$c6502_do = false
$WIN_do = true

#Compute maximal lenght of a name
$name_len = 0
$tests_to_run.each { |name|
	if name.size > $name_len
		$name_len = name.size
	end
}
	
$tests_to_run.each { |name|
	$test_count += 1
	print "#{name.ljust($name_len, " ")}"

	test_ok = true

	if c6502_do		
		print " 6502:"
		if test(name, "con6502") then
			print green("OK")
			$c6502_OK += 1
		else
			test_ok = false
			print red("XX")
		end
	end
	
	if Z80_do
		print " Z80:"
		if test(name, "conZ80") then
			$Z80_OK += 1
			print green("OK")                         	
		else
			test_ok = false
			print red("XX")
		end
	end
	
	if WIN_do
		print " WIN:"
		if test(name, "windows") then
			$WIN_OK += 1
			print green("OK")
		else
			test_ok = false
			print red("XX")
		end
	end
		
	if test_ok then
		$test_ok += 1
	end
		
	puts	 
}

puts
puts "Tests   OK: #{$test_ok}  Failed: #{$test_count - $test_ok}"
puts "6502    OK: #{$c6502_OK}  Failed: #{$test_count - $c6502_OK}"
puts "Z80     OK: #{$Z80_OK}  Failed: #{$test_count - $Z80_OK}"
puts "windows OK: #{$WIN_OK}  Failed: #{$test_count - $WIN_OK}"

