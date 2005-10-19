require 'Sample'

# abstract base class of all estimators

class Estimator

    # processes a sample
    def process(sample)
    end
    
    # returns an estimate (ETA as float)
    # note that you must process at least one sample before this will return meaningful output
    def estimate
    end

    # returns the name of the estimator
    def name
    end
end

# estimator that uses the current speed
class CSAEstimator < Estimator
    def process(sample)
        @sample = sample.clone
    end
    
    def estimate
        return @sample.bytesLeft.to_f / @sample.speed
    end

    def name
        'CurrentSpeedEstimator'
    end
end

# estimator that uses the global average speed of the whole torrent download for estimation

class GASAEstimator < Estimator
    def process(sample)
        @first = sample.clone if @first == nil
        @last = sample.clone
        @avgSpeed = Sample.averageSpeed(@first, @last)
    end

    def estimate
        return @last.bytesLeft.to_f / @avgSpeed
    end

    def name
        'AverageSpeedEstimator'
    end
end

# estimator that uses the average over the last n seconds

class WINXEstimator < Estimator

    attr_reader :windowSize

    def process(sample)
        # remove all samples that are older than the window size. Note: samples are sorted.
        @list.pop until @list.length <= 1 or (sample.time - @list.last.time) <= @windowSize

        # prepend array with newest sample
        @list.unshift(sample.clone)
    end

    def estimate

        if @list.length > 1
            first = @list.first
            last = @list.last
            return first.bytesLeft.to_f / Sample.averageSpeed(last, first) 
        elsif @list.length == 1
            sample = @list.first
            return  sample.bytesLeft.to_f / sample.speed
        elsif @list.length == 0
            return 0
        end
    end
    
    def name
        "MovingAverageEstimator_#{@windowSize}s"
    end

    def initialize(windowSizeInSeconds)
        @list = Array.new
        @windowSize = windowSizeInSeconds
    end
end
