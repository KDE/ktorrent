
class Sample

    attr_reader :time, :speed, :bytesDownloaded, :bytesLeft, :peersTotal

    def Sample.averageSpeed(sample1, sample2)
        if sample2.time - sample1.time > 0 
            return (sample1.bytesLeft - sample2.bytesLeft).to_f / (sample2.time - sample1.time).to_f
        else 
            return sample1.speed
        end
    end

    def <=>(other)
        @time <=> other.time
    end

    # parses a single sample from a line. Format is 
    #
    #   \<tt>timestamp,speed,bytesDownloaded,bytesLeft,peersTotal</tt>
    #
    #   where 
    #    - timestamp is in seconds since epoch (Integer)
    #    - speed is bytes/seconds as Integer
    #    - bytesDownloaded, bytesLeft are bytes as Integer
    #    - peersTotal is the number of available peers (both seeders and leecher, both
    #      connected and not connected to us)

    def Sample.parse(line)

        splitted = line.split(",")
        
        # TODO: do better error checking
        return nil if splitted.length != 5

        time = splitted[0].to_i
        speed = splitted[1].to_i
        bytesDownloaded = splitted[2].to_i
        bytesLeft = splitted[3].to_i
        peersTotal = splitted[4].to_i
        return Sample.new(time, speed, bytesDownloaded, bytesLeft, peersTotal)
    end

    # parses samples from a text file, with one sample per line
    def Sample.parseFromFile(filename)
        samples = Hash.new
        
        input = File.open(filename)
        input.each_line do |line|
            s = Sample.parse(line)
            samples[s.time] = s unless s == nil
        end
        input.close
        return samples
    end

    def initialize(time, speed, bytesDownloaded, bytesLeft, peersTotal)
        @time = time
        @speed = speed
        @bytesDownloaded = bytesDownloaded
        @bytesLeft = bytesLeft
        @peersTotal = peersTotal
    end
end
