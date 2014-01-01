#*** Register Arguments (1)

def load_file(filename)
# Read text file (eventually removing UTF-8 BOM chars and return it as list of lines
  arr = IO.readlines(filename, :encoding => 'bom|UTF-8')
  if arr[0] =~ /\A\xEF\xBB\xBF(.*)/
    arr[0] = $1
  end
  return arr
end

def find_info(header)
# Collect steps from source files in the format
#  // *** {Text} ({step})
#  //     Description lines..
 	
	info = Hash[]
	
	Dir.glob('src/atalan/*.c') do |rb_file|
		# do work on files ending in .rb in the desired directory
#		puts rb_file

		f = load_file(rb_file)

		section_no = 0			  
		line_no = 0
		text = ""
		f.each { |line|
			line_no += 1
			
			if line =~ /\s*\/\/\s*\*\*\*\s*#{header}/				 	
		        mt = line.match(/\s*\/\/\s*\*\*\*\s*#{header}\s*\((\d*)\)/)
				section_no = mt[1].to_i
#				puts "#{line_no} #{mt[1]} #{line}"
				
			elsif line =~ /\s*\/\//
				if section_no != 0
			        mt = line.match(/\s*\/\/\s*(.*)/)			        
					text = text + " " + mt[1]
				end
			else
				if section_no != 0
					info[section_no] = { :text => text, :file => rb_file, :line_no => line_no }
#					puts "#{section_no} #{text}" 
				end
				section_no = 0
				text = ""
			end
		}
	end
	
	puts header
	puts ""
	info.sort.each{|section_no, d| puts "#{section_no}. #{d[:text]}" }
	puts ""	  
end

find_info("Register Arguments")
find_info("Module parameters")
find_info("Type Assert")
