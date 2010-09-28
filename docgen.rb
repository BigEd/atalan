# Famulus Doc Generator

$proj_folder = "P:/Atalan/"

def put_file(filename)
  head = IO.readlines($proj_folder + "www_src/" + filename) 
  head.each{|line|
    puts line
  }
end

def html_file(filename)
  put_file("header.html")
  put_file(filename)
  put_file("footer.html") 
end

def generate_index(filename)

  arr = IO.readlines($proj_folder + filename)
  
  heading_level = 0
  lv = [0, 0, 0, 0]
  first = true
  text = "<div id=\"index\">\n"
  arr.each_with_index { |line, index|  
    if line =~ /\*\*+/ 
      if heading_level == 1
        heading_level = 0
        first = false
      else
        heading_level = 1
      end
    elsif line =~ /\A\=\=+/ 
      if heading_level == 2
        heading_level = 0
      else
        heading_level = 2
      end
    else
      if heading_level > 0 and not first
        lv[heading_level] = lv[heading_level] + 1
        text = text + "<h#{heading_level}><a href=\"\##{lv[1]}_#{lv[2]}\">#{line.strip}</a></h#{heading_level}>\n"
      end
    end
  }
  text = text + "</div>\n"
  return text
end

def a_file(filename, in_comment)

  index_text = generate_index(filename)
  
  arr = IO.readlines($proj_folder + filename)
  
  lv = [0, 0, 0, 0]
  in_list = false
  in_code = false
  first = true
  
  heading_level = 0
  arr.each_with_index { |line, index|
    # Non Comment section
    if not in_comment
      if line =~ /\A\/\*\Z/
        in_comment = true
      end
    # Comment section
    else
      if line.strip == "::index::"
        puts index_text
      elsif line =~ /\A\*\/\Z/
        in_comment = false
      elsif line =~ /\A:::::+\s*\Z/
        if not in_code
          puts "<code>"
          in_code = true
        else
          puts "</code>"
          in_code = false
        end
          
      elsif line =~ /\A\s*\-/
        if not in_list then
          puts "<ul>"
          in_list = true
        end
        mt = line.match(/\A\s*-\s*(.*)\Z/)
        puts '<li>'
        puts mt[1]
        puts '</li>'
      elsif line =~ /\*\*+/ 
        if heading_level == 1
          heading_level = 0
          first = false
        else
          heading_level = 1
        end
      elsif line =~ /\A\=\=+/ 
        if heading_level == 2
          heading_level = 0
        else
          heading_level = 2
        end
      elsif line =~ /Purpose:/
        #in header file, two lines back should be function declaration like "typedef void   (*DataFreeFn)   (VoidPtr data);"
        mt = arr[index-2].match(/typedef\s+(\w+)\s+\(\*(.+)Fn\)\s*\((.*)\);/)
        puts
        puts  "<h3 class=\"func\">#{mt[1]} <b>#{mt[2]}</b>(#{mt[3]})</h3>"
        puts
      elsif line =~ /\A\s*\Z/
          if in_list then
            in_list = false
            puts "</ul>"
          else
          	if in_code then
            	puts ""
            else
            	puts "</p><p>"
            end
          end
      else
        if heading_level > 0
          if first
            puts "<h#{heading_level}>#{line.strip}</h#{heading_level}>"
          else
            lv[heading_level] = lv[heading_level] + 1
            puts "<a name=\"#{lv[1]}_#{lv[2]}\"><h#{heading_level}>#{line.strip}</h#{heading_level}>"
          end
        else
          if in_code then
            puts line
          else
            puts line.strip
          end
          
        end
      end
    end    
  }

  
end

def o_file(filename, in_comment)
  #Generate header
  put_file("header.html")

  a_file(filename, in_comment)
  #Generate footer  
  put_file("footer.html") 
end

def h_file(filename)
    o_file(filename, false)
end

def  txt_file(filename)
  o_file("doc/" + filename, true)
end

def ata_example(filenames)
  put_file("header.html")
  a_file("doc/" + "examples.txt", true)
  puts "<table>"
  filenames.each { |filename|
    puts "<tr>"
    puts "  <td>#{filename}</td><td><a href=\"examples/#{filename}.atl\">#{filename}.atl</a></td>"
    puts "  <td><a href=\"examples/#{filename}.asm\">#{filename}.asm</a></td>"
    puts "  <td><a href=\"examples/#{filename}.xex\">#{filename}.xex</a></td>"
    puts "</tr>"
  }
  puts "</table>"
  put_file("footer.html") 
end

$stdout = File.new($proj_folder + 'www/index.html', 'w')
txt_file "index.txt"

$stdout = File.new($proj_folder + 'www/contact.html', 'w')
txt_file "contact.txt"

$stdout = File.new($proj_folder + 'www/reference.html', 'w')
txt_file "reference.txt"

$stdout = File.new($proj_folder + 'www/examples.html', 'w')
#txt_file "examples.txt"
ata_example ["hello_world","loop","unit_test","hello_font","rainbow","esieve","count_2sec","stars","tetris","interrupts","bit_sieve"]



$stdout = File.new($proj_folder + 'www/tutorial.html', 'w')
txt_file "tutorial.txt"

$stdout = File.new($proj_folder + 'www/usage.html', 'w')
txt_file "usage.txt"

$stdout = File.new($proj_folder + 'www/optim.html', 'w')
txt_file "optim.txt"

$stdout = File.new($proj_folder + 'www/backend.html', 'w')
txt_file "backend.txt"

$stdout = File.new($proj_folder + 'www/atari.html', 'w')
txt_file "platform_atari.txt"

$stdout = File.new($proj_folder + 'www/news.html', 'w')
txt_file "news.txt"

$stdout = File.new($proj_folder + 'www/projects.html', 'w')
txt_file "projects.txt"
