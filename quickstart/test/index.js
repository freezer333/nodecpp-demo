const addon = require('my_addon');
const assert = require('assert');

assert(addon.pass_number(100) == 142);
assert(addon.pass_number(100.5) == 142.5);
assert(addon.pass_number("100") == 142);
assert(isNaN(addon.pass_number("no")));
assert(isNaN(addon.pass_number()));
assert(isNaN(addon.pass_number(function(){})));

assert(addon.pass_integer(5) == 47);
assert(addon.pass_integer(17.23) == undefined);
assert(addon.pass_integer() == undefined);

assert(addon.pass_boolean(false));
assert(addon.pass_boolean(true) == false);
assert(addon.pass_boolean() == undefined);
assert(addon.pass_boolean("no") == undefined);
assert(addon.pass_boolean(1) == undefined);


assert(addon.pass_string("hello") == "olleh");
assert(addon.pass_string() == undefined);
assert(addon.pass_string(88) == undefined);

var check = addon.pass_object({x: 5, y: 6});
assert(check)
assert(check.sum == 11);
assert(check.product == 30);

check = addon.pass_object();
assert(!check);

check = addon.pass_object({foo:5, bar:6});
assert(isNaN(check.sum));
assert(isNaN(check.product));

check = addon.pass_object({x:"6", y:"5"});
assert(check.sum == 11);
assert(check.product == 30);


var inputs = [1, 2, 3];
var squares = addon.pass_array(inputs);
assert(inputs[0] == 2);
assert(inputs[1] == 3);
assert(inputs[2] == 4);
assert(squares[0] == 1);
assert(squares[1] == 4);
assert(squares[2] == 9);
assert(squares.sum_of_squares == 14);

var obj = {
    a: 5,
    b : [1, 2, 3]
}

addon.add_array(obj);
assert(obj.b[0] == 6);
assert(obj.b[1] == 7);
assert(obj.b[2] == 8);

console.log("Hooray!  The addon worked as expected.");