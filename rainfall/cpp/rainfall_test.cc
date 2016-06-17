#include <iostream>
#include "rainfall.h"
using namespace std;

int main() {
  location loc;

  loc.latitude = 40.71;
  loc.longitude = -74.01;

  loc.samples.push_back(sample("2014-11-30", 1.00));
  loc.samples.push_back(sample("2014-12-01", 1.50));
  loc.samples.push_back(sample("2014-12-02", 0.25));

  cout << "Average rainfall = " << avg_rainfall(loc) << "cm" << endl;
}
