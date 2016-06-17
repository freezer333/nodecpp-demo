const addon = require('./build/Release/primes');

addon.primes_progress(20, function (primes) {
    // prints 2, 3, 5, 7, 11, 13, 17, 19
    console.log("Primes less than 20 = " + primes);
}, function(progress) {
    console.log(progress + "% complete");
}); 