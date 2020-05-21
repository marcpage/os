#ifndef __Statistics_h__
#define __Statistics_h__

#include <algorithm> // std::accumulate
#include <math.h>
#include <numeric>
#include <stdexcept> // std::out_of_range
#include <string>
#include <vector>

namespace math {
typedef std::vector<double>
    List; ///< A list of double-precision floating point numbers

/** Get the sum of a list of numbers.
        @param numbers list of numbers to add up.
        @return the sum of the numbers
*/
inline double sum(const List &numbers) {
  return std::accumulate(numbers.begin(), numbers.end(), 0);
}

/** Get the mean (or average) of a list of numbers.
        @param numbers the list of numbers to get the mean
        @return the mean of the numbers
        @throws std::out_of_range if there are no numbers in numbers
*/
inline double mean(const List &numbers) {
  if (numbers.size() < 1) {
    throw std::out_of_range(
        "You need at least one value to get mean, we only have " +
        std::to_string(numbers.size()));
  }

  return math::sum(numbers) / double(numbers.size());
}

/** Get various statistics about a list of numbers.
        @param numbers the numbers to evaluate
        @param mean receives the mean (or average) of the numbers
        @param sum receives the sum of the numbers
        @param variance sum of the square of the distance between each number
   and the mean divided by one less than the number of items in numbers
        @param standardDeviation square root of the variance
        @throws std::out_of_range if there are not at least two items in numbers
*/
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

  for (auto n : numbers) {
    // Consider using std::accumulate algorithm instead of a raw loop
    // cppcheck-suppress useStlAlgorithm
    distanceSquaredSum += (n - mean) * (n - mean);
  }
  variance = distanceSquaredSum / double(numbers.size() - 1);
  standardDeviation = sqrt(variance);
}

/** Get the variance for a list of numbers.
        @param numbers the number to evaluate
        @return sum of the square of the distance between each number and the
   mean divided by one less than the number of items in numbers
        @throws std::out_of_range if there are not at least two items in numbers
*/
inline double variance(const List &numbers) {
  double meanValue = 0.0, sumValue = 0.0, varianceValue = 0.0,
         standardDeviationValue = 0.0;
  statistics(numbers, meanValue, sumValue, varianceValue,
             standardDeviationValue);

  return varianceValue;
}

/** Get the standard deviation of a list of numbers.
        @param numbers the number to evaluate
        @return the standard deviation of a list of numbers.
        @throws std::out_of_range if there are not at least two items in numbers
*/
inline double stddev(const List &numbers) {
  double meanValue = 0.0, sumValue = 0.0, varianceValue = 0.0,
         standardDeviationValue = 0.0;
  statistics(numbers, meanValue, sumValue, varianceValue,
             standardDeviationValue);

  return standardDeviationValue;
}

inline List &filterInRange(List &numbers, double min, double max) {
  for (int i = 0; i < int(numbers.size()); ++i) {
    if ((numbers[i] < min) || (numbers[i] > max)) {
      numbers.erase(numbers.begin() + i, numbers.begin() + i + 1);
      --i;
    }
  }

  return numbers;
}

inline double min(const List &numbers) {
  auto smaller = [](double a, double b) { return std::min(a, b); };

  if (numbers.size() == 0) {
    return 0.0;
  }

  return std::accumulate(numbers.begin(), numbers.end(), numbers[0], smaller);
}

inline double max(const List &numbers) {
  auto larger = [](double a, double b) { return std::max(a, b); };

  if (numbers.size() == 0) {
    return 0.0;
  }

  return std::accumulate(numbers.begin(), numbers.end(), numbers[0], larger);
}

} // namespace math

#endif // __Statistics_h__
