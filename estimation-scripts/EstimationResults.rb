require 'Estimators'
require 'Sample'

class EstimationResults

    attr_reader :estimator
    
    def initialize(estimator, samples)
        @samples = samples
        @totalTime = samples.keys.max
        @estimator = estimator
        
        @maxError = nil
        @estimations = nil
        @absoluteErrors = nil
        @relativeErrors = nil
        @rootMeanSquareErrorRelative = nil
    end

    def getRootMeanSquareErrorRelative
       if @rootMeanSquareErrorRelative == nil 
           relativeErrors = getRelativeErrors
           @rootMeanSquareErrorRelative = 0.0;
           relativeErrors.each_value do |x|
               @rootMeanSquareErrorRelative += x**2
           end
           @rootMeanSquareErrorRelative = Math.sqrt( @rootMeanSquareErrorRelative / relativeErrors.size )
       end
       return @rootMeanSquareErrorRelative

    end

    def getRelativeErrors
         if @relativeErrors == nil
            @relativeErrors = Hash.new
            absoluteErrors = getAbsoluteErrors
            absoluteErrors.keys.sort.each do |time|
                timeLeft = @totalTime - time;
                @relativeErrors[time] = absoluteErrors[time].abs.to_f / timeLeft
                @relativeErrors[time] = @maxError if @maxError != nil and @relativeErrors[time] > @maxError
            end
         end
         return @relativeErrors
    end

    def setMaxError(maxError)
        if maxError != @maxError
            @maxError = maxError
            @relativeErrors = nil
            @rootMeanSquareErrorRelative = nil
        end
    end

    def getAbsoluteErrors
         if @absoluteErrors == nil
             @absoluteErrors = Hash.new
             estimations = getEstimations
             estimations.keys.sort.each do |time|
                 @absoluteErrors[time] = @estimations[time] - (@totalTime - time)
             end
         end

         return @absoluteErrors
    end

    def getEstimations
        
        if @estimations == nil
            @estimations = Hash.new
            @samples.values.sort.each do |sample|
                @estimator.process(sample)
                @estimations[sample.time] = @estimator.estimate
            end
        end

        return @estimations
    end

     
end

