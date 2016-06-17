var rainfall = require("./cpp/build/Release/rainfall");
var location = {
    latitude : 40.71, longitude : -74.01,
       samples : [
          { date : "2015-06-07", rainfall : 2.1 },
          { date : "2015-06-14", rainfall : 0.5},
          { date : "2015-06-21", rainfall : 1.5},
          { date : "2015-06-28", rainfall : 1.3},
          { date : "2015-07-05", rainfall : 0.9}
       ] };

// utility printing
var print_rain_results = function(results) {
  var i = 0;
  results.forEach(function(result){
      console.log("Result for Location " + i);
      console.log("--------------------------");
      console.log("\tLatitude:         " + locations[i].latitude.toFixed(2));
      console.log("\tLongitude:        " + locations[i].longitude.toFixed(2));
      console.log("\tMean Rainfall:    " + result.mean.toFixed(2) + "cm");
      console.log("\tMedian Rainfall:  " + result.median.toFixed(2) + "cm");
      console.log("\tStandard Dev.:    " + result.standard_deviation.toFixed(2) + "cm");
      console.log("\tNumber Samples:   " + result.n);
      console.log();
      i++;
  });
}

// Part 1
console.log("Average rain fall = " + rainfall.avg_rainfall(location) + "cm");

// Part 2
console.log("Rainfall Data = " + JSON.stringify(rainfall.data_rainfall(location)));

// Part 3

var makeup = function(max) {
    return Math.round(max * Math.random() * 100)/100;
}

var locations = []
for (var i = 0; i < 10; i++ ) {
    var loc = {
        latitude: makeup(180),
        longitude: makeup(180),
        samples : [
            {date: "2015-07-20", rainfall: makeup(3)},
            {date: "2015-07-21", rainfall: makeup(3)},
            {date: "2015-07-22", rainfall: makeup(3)},
            {date: "2015-07-23", rainfall: makeup(3)}
        ]
    }
    locations.push(loc);
}

var results = rainfall.calculate_results(locations);
print_rain_results(results);


// Part 4 - calling asynchronous c++ addon
rainfall.calculate_results_async(locations, 
  function(err, result) {
    if (err ) {
      console.log(err);
    }
    else {
      print_rain_results(result);
    }
    
  });

console.log("Async results probably still not here yet...")
