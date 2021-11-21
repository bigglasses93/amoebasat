flags = File::CREAT |File::RDWR

#ARGV[0]=program name
#ARGV[1]=opt
#ARGV[2]=debug
$mem = Array.new
$mem_new = Array.new
$var_count = 0
$clause_count = 0
$clause_k = Array.new
$d = ARGV[2]
$cl_cnt = 0

g = ARGV[0].split(/_/)
gn = File.basename(ARGV[0], ".cnf")

#read input, count initial vars#
File.open("#{ARGV[0]}", 'r'){|f|
	while inline = f.gets
		if inline =~ /^c/
			next
		elsif inline =~ /^p/
			tmp = inline.chomp.split(/ /)
			$var_count = tmp[2].to_i
			$clause_count = tmp[3].to_i 
		elsif inline != nil
			$mem << inline.chomp.chop			
			a = inline.chomp.split(/ /)
			#find max variable id
			a.each{|y|
				if y.to_i.abs > $var_count 
					$var_count = y.to_i.abs
				end
			}
		end
	end
}

puts "initial var_count =  #{$var_count}"
#puts $mem

#convert to 3-SAT
for k in 0..$mem.size-1
	y = $var_count
	clause = $mem[k].split(/ /)
	if clause.size <= 3
		$mem_new << $mem[k]
		$cl_cnt = $cl_cnt+1
	else 
		#make the first 2 indices
		$mem_new << "#{clause[0]} #{clause[1]} #{y+1}"
		y = y+1
		$cl_cnt = $cl_cnt+1
		#make the in-between indices
		for i in 2..clause.size-3
			clause_new = "-#{y} #{clause[i]} #{y+1}"
			y = y+1
			$cl_cnt = $cl_cnt+1
			$mem_new << clause_new
		end
		#make the last 2 indices
		$mem_new << "-#{y} #{clause[clause.size-2]} #{clause[clause.size-1]}"
		$var_count = y
		$cl_cnt = $cl_cnt+1
	end
end
#puts $mem_new
puts "output var_count = #{$var_count}"
puts "output clause_count = #{$cl_cnt}"
outfile = File.open("#{gn.chomp(".cnf.txt")}_out.cnf", "w")
outfile.puts "p cnf #{$var_count} #{$cl_cnt}"
outfile.puts $mem_new