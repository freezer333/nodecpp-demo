'use strict';
const fs = require('fs');
const path = require('path');
const png2bmp = require('./build/Release/png2bmp');
var png_file = process.argv[2];
var bmp_file = path.basename(png_file, '.png') + ".bmp";
var png_buffer = fs.readFileSync(png_file);

/* synchronous version...
var bmp_buffer = png2bmp.getBMP(png_buffer, png_buffer.length);
fs.writeFileSync(bmp_file, bmp_buffer);
*/

png2bmp.getBMPAsync(png_buffer, png_buffer.length, function(err, bmp_buffer) {
    if (err) {
        console.log(err);
    }
    else {
        fs.writeFileSync(bmp_file, bmp_buffer);
    }
}); 
