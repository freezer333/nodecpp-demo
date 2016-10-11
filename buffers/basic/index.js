'use strict';
const addon = require('./build/Release/buffer_example');

const buffer = Buffer.from("ABC");

// synchronous, rotates each character by +13
addon.rotate(buffer, buffer.length);

