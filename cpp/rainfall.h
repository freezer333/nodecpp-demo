#include <vector>
#include <string>
using namespace std;

class sample {
public:
  sample () {
    date = ""; rainfall = 0;
  }
  sample (string d, double r) {
    date = d;
    rainfall = r;
  }
  string date;
  double rainfall;
};

class location {
public:
  double longitude;
  double latitude;
  vector<sample> samples;
};


class rain_result {
   public:
       float median;
       float mean;
       float standard_deviation;
       int n;
};

double avg_rainfall(location & loc);

rain_result calc_rain_stats(location &loc);
