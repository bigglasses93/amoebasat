gn = File.basename(ARGV[0], ".cnf")

outfile = File.open("#{gn}2.cnf", "w")

parray = Array.new
File.open("#{ARGV[0]}", 'r'){|f|
	while inline = f.gets
		str = inline.chomp.split(/ /)
		ps = 4 - str.size
		ps.times{str << "0"}
		parray << str.join(" ")
	end
}

cnt = 0
max_var = 0
for i in 0..parray.size-1
	outfile.print "#{parray[i]}"
	if i == parray.size-1
		tmp = parray[i].split(/ /)
		tmp.each{|x|
			if x.include?("-")
				x =~ /\-(.+)/
				y = $+.to_i
			else
				y = x.to_i
			end
			max_var = y if y > max_var
		}
	else
		cnt += 1
		outfile.puts 
	end
end
