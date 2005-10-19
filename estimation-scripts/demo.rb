require 'Sample'
require 'Estimators'
require 'EstimationResults'

samples = Sample.parseFromFile(ARGV[0])

est = WINXEstimator.new(ARGV[1].to_i)

results = EstimationResults.new(est, samples)
results.setMaxError(10.0)

relErrors = results.getRelativeErrors

relErrors.keys.sort.each do |x|
    puts "#{x} #{relErrors[x]}"
end

#puts "RMSE: #{results.getRootMeanSquareErrorRelative}"
