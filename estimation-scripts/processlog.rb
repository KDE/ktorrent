IDX_TIME = 0
IDX_STATE = 5

def adjustTimestamps(perFile)
    startTime = 0
    offset = 0
    lastDeactivation = -1
    lastSample = nil
    
    perFile.each_key do |file|
        perFile[file].each do |line|
    
            time = line[0].to_i
    
            startTime = time if startTime == 0
            
            time = time - startTime - offset
                
            line[IDX_TIME] = time.to_s
            
            if line[IDX_STATE] == 'RUNNING'
                lastSample = line
            elsif line[IDX_STATE] == 'ACTIVATED'
                offset = time - lastDeactivation unless lastDeactivation == -1
                perFile[file].delete(line)
            elsif line[IDX_STATE] == 'DEACTIVATED'
                lastDeactivation = time
                perFile[file].delete(line)
            elsif line[IDX_STATE] == 'FINISHED'
                # print last sample: time speed=0 downloaded left=0 peersTotal
                # puts "#{line[0].to_i},0,#{lastSample[2].to_i + lastSample[3].to_i},0,#{lastSample[4].to_i}"
                perFile[file].delete(line)
            end
        end
    end
end

perFile = Hash.new

inputFile = File.new(ARGV[0])

inputFile.each do |line|

   splitted = line.strip.split(",")
   if splitted.length == 7
       key = splitted[0]
       perFile[key] = Array.new if perFile[key] == nil
       perFile[key].push(splitted[1..6]) 
   end

end

inputFile.close

adjustTimestamps(perFile)

perFile.each_key do |file|
    outfile = File.new("torrent-#{file}.log", "w")
    perFile[file].each do |line|
        outfile.puts line[0..4].join(",")
    end
    outfile.close
end
