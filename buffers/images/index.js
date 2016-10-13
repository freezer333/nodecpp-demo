'use strict';
const fs = require('fs');
const path = require('path');
const png2bmp = require('./build/Release/png2bmp');
const png_file = process.argv[2];
const bmp_file = path.basename(png_file, '.png') + ".bmp";
const png_buffer = fs.readFileSync(png_file);

// Synchronous version
//const bmp_buffer = png2bmp.getBMP(png_buffer, png_buffer.length);
//fs.writeFileSync(bmp_file, bmp_buffer);  

png2bmp.getBMPAsync(png_buffer, png_buffer.length, function(err, bmp_buffer) {
    if (err) {
        console.log(err);
    }
    else {
        fs.writeFileSync(bmp_file, bmp_buffer);
    }
}); 
