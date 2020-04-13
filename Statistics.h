#ifndef __Statistics_h__
#define __Statistics_h__

#include <math.h>
#include <string>
#include <vector>

namespace math {
typedef std::vector<double> List;

inline double sum(const List &numbers) {
  double sum = 0.0;

  for (List::const_iterator n = numbers.begin(); n != numbers.end(); ++n) {
    sum += *n;
  }

  return sum;
}

inline double mean(const List &numbers) {
  if (numbers.size() < 1) {
    throw std::out_of_range(
        "You need at least one value to get mean, we only have " +
        std::to_string(numbers.size()));
  }

  return math::sum(numbers) / double(numbers.size());
}

inline void statistics(const List &numbers, double &mean, double &sum,
                       double &variance, double &standardDeviation) {
  double distanceSquaredSum = 0.0;

  if (numbers.size() < 2) {
    throw std::out_of_range(
        "You need at least two values to get statistics, we only have " +
        std::to_string(numbers.size()));
  }

  sum = math::sum(numbers);
  mean = sum / double(numbers.size());

  for (List::const_iterator n = numbers.begin(); n != numbers.end(); ++n) {
    distanceSquaredSum += (*n - mean) * (*n - mean);
  }

  variance = distanceSquaredSum / double(numbers.size() - 1);
  standardDeviation = sqrt(variance);
}

inline double variance(const List &numbers) {
  double meanValue = 0.0, sumValue = 0.0, varianceValue = 0.0,
         standardDeviationValue = 0.0;
  statistics(numbers, meanValue, sumValue, varianceValue,
             standardDeviationValue);

  return varianceValue;
}

inline double stddev(const List &numbers) {
  double meanValue = 0.0, sumValue = 0.0, varianceValue = 0.0,
         standardDeviationValue = 0.0;
  statistics(numbers, meanValue, sumValue, varianceValue,
             standardDeviationValue);

  return standardDeviationValue;
}

} // namespace math

#endif // __Statistics_h__
