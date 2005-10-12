require 'Sample'
require 'Estimators'
require 'EstimationResults'

samples = Hash.new

input = File.open(ARGV[0])
input.each_line do |line|
    s = Sample.parse(line)
    samples[s.time] = s
end
input.close

est = WINXEstimator.new(100)

results = EstimationResults.new(est, samples)
results.setMaxError(5)

relErrors = results.getRelativeErrors

relErrors.keys.sort.each do |x|
    puts "#{x} #{relErrors[x]}"
end

#puts "RMSE: #{results.getRootMeanSquareErrorRelative}"
