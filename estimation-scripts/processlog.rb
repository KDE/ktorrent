perFile = Hash.new

inputFile = File.new(ARGV[0])

inputFile.each do |line|

   splitted = line.split(",")
   if splitted.length == 7
       key = splitted[0]
       perFile[key] = Array.new if perFile[key] == nil
       perFile[key].push(splitted[1..6]) 
   end

end

inputFile.close

perFile.each_key do |file|
    outfile = File.new("torrent-#{file}.log", "w")
    perFile[file].each do |line|
        outfile.puts line.join(",")
    end
    outfile.close
end
