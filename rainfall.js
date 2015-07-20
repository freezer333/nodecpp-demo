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
console.log(JSON.stringify(results));
