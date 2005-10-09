IDX_TIME = 0
IDX_STATE = 5

startTime = 0
offset = 0
lastDeactivation = -1

lastSample = nil

aFile = File.new(ARGV[0], "r")
aFile.each do |line|

   splitted = line.split(",")
   time = splitted[0].to_i

   startTime = time if startTime == 0
   
   time = time - startTime - offset
     
   splitted[IDX_TIME] = time.to_s
   splitted[IDX_STATE].strip!

   if splitted[IDX_STATE] == 'RUNNING'
        lastSample = splitted
        puts splitted[0..4].join(",")
   elsif splitted[IDX_STATE] == 'ACTIVATED'
           offset = time - lastDeactivation unless lastDeactivation == -1
   elsif splitted[IDX_STATE] == 'DEACTIVATED'
           lastDeactivation = time
   elsif splitted[IDX_STATE] == 'FINISHED'
           # print last sample: time speed=0 downloaded left=0 peersTotal
           # puts "#{splitted[0].to_i},0,#{lastSample[2].to_i + lastSample[3].to_i},0,#{lastSample[4].to_i}"
    end

end

aFile.close
