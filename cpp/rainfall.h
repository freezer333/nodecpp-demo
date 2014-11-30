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

double avg_rainfall(location & loc);
