#include "rainfall.h"

double avg_rainfall(location & loc) {
  double total = 0;
  for (const auto &sample : loc.samples) {
    total += sample.rainfall;
  }
  return total / loc.samples.size();
}

rain_result calc_rain_stats(location &loc) {
    return rain_result();
}