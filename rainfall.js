var rainfall = require("./cpp/build/Release/rainfall.node");
var location = {
    latitude : 40.71, longitude : -74.01,
       samples : [
          { date : "2014-06-07", rainfall : 2 },
          { date : "2014-08-12", rainfall : 0.5}
       ] };

console.log("Average rain fall = " + rainfall.avg_rainfall(location) + "cm");
