const loose = require('./loose/build/Release/loose_type_demo');
const strict = require('./strict/build/Release/strict_type_demo');

var assert = require('assert');


function reverse(s) {
  return s.split('').reverse().join('');
}

describe('loose conversions', function() {

  /* pass_number accepts any numeric value and adds 42 to it.
     pass_integer accepts an integer value and adds 42 to it.
     loose intepretation of numbers in V8 mimic's exactly what 
     we'd expect in JavaScript proper.
  */
  describe('pass_number()', function () {
    it('should return input + 42 when given a valid number', function () {
      assert.equal(23+42, loose.pass_number(23));
      assert.equal(0.5+42, loose.pass_number(0.5));
    });

    it('should return input + 42 when given numeric value as a string', function () {
      assert.equal(23+42, loose.pass_number("23"));
      assert.equal(0.5+42, loose.pass_number("0.5"));
    });

    it('should return 42 when given null (null converts to 0)', function () {
      assert.equal(42, loose.pass_number(null));
    });

    it('should return NaN when given undefined (undefined converts to NaN)', function () {
      assert(isNaN(loose.pass_number()));
      assert(isNaN(loose.pass_number(undefined)));
    });

    it('should return NaN when given a string that cannot be parsed to number', function () {
      assert(isNaN(loose.pass_number("this is not a number")));
    });

    it('should return 42 when given an empty array ([] converts to 0)', function () {
      assert(isNaN(loose.pass_number({})));
    });

    it('should return NaN when given a non-empty object/array', function () {
      assert(isNaN(loose.pass_number({})));
      assert(isNaN(loose.pass_number({x: 5})));
      assert(isNaN(loose.pass_number([1, 2])));
    });

  });





  describe('pass_integer()', function () {
  	it('should return input + 42 when given integer value', function () {
      assert.equal(5+42, loose.pass_integer(5));
    });
    it('should return Math.floor(input) + 42 when given floating point value', function () {
      assert.equal(Math.floor(5)+42, loose.pass_integer(5.7));
    });
  });




  describe('pass_string()', function () {
  	/* pass_string accepts any string value and returns the reverse of it.
       loose intepretation of strings in V8 mimic's exactly what 
       we'd expect in JavaScript proper.
    */
    
  	it('should return reverse a proper string', function () {
      assert.equal(reverse("The truth is out there"), loose.pass_string("The truth is out there"));
    });
    it('should return reverse a numeric since numbers are turned into strings', function () {
      assert.equal("24", loose.pass_string(42));
      assert.equal("eurt", loose.pass_string(true));
    });
    it('should return "llun" when given null - null is turned into "null"', function () {
      assert.equal("llun", loose.pass_string(null));
    });
    it('should return "denifednu" when given undefined - null is turned into "undefined"', function () {
      assert.equal(reverse("undefined"), loose.pass_string(undefined));
    });
    it('should return reverse of object serialized to string when given object', function () {
      assert.equal(reverse("[object Object]"), loose.pass_string({x: 5}));
    });
    it('should return reverse of array serialized to string when given array', function () {
      assert.equal(reverse("9,0"), loose.pass_string([9, 0]));
    });
  });


  describe('pass_boolean()', function () {
  	/* pass_boolean accepts any boolean value and returns the COMPLIMENT.
       loose intepretation of booleans in V8 mimic's exactly what 
       we'd expect in JavaScript proper.
    */
    
  	it('should return compliment of proper boolean', function () {
      assert(loose.pass_boolean(true) == false);
      assert(loose.pass_boolean(false) == true);
    });

    it('should return false if given anything considered "truthy', function () {
      assert(loose.pass_boolean(42) == false);
      assert(loose.pass_boolean("is this true?") == false);
      assert(loose.pass_boolean({}) == false);
      assert(loose.pass_boolean({x: 5}) == false);
      assert(loose.pass_boolean([]) == false);
      assert(loose.pass_boolean([1, 2]) == false);
    });

    it('should return true if given anything considered "falsey', function () {
      assert(loose.pass_boolean(0));
      assert(loose.pass_boolean(undefined));
      assert(loose.pass_boolean(null));
    });
    
  });

  describe('pass_object()', function () {
    /* pass_object accepts any object and extracts numeric x/y properties.  It
       then constructs a new object with the sum and product of x/y.  It
       demonstrates how properties can be handled with objects in V8.
    */

    it('should fully compute properties given object with required properties', function () {
      assert.equal(13, loose.pass_object({x:3, y:10}).sum);
      assert.equal(30, loose.pass_object({x:3, y:10}).product);
    });
    it('should set sum and product to NaN given object without x or y', function () {
      assert(13, isNaN(loose.pass_object({y:10}).sum));
      assert(30, isNaN(loose.pass_object({x:3}).product));
      assert(isNaN(loose.pass_object({x:3, y:"hello"}).product));
    });
    it('should fully compute properties given object with required properties - even when not pure numerics', function () {
      assert.equal(4, loose.pass_object({x:3, y:true}).sum);
      assert.equal(30, loose.pass_object({x:3, y:"10"}).product);
      assert.equal(0, loose.pass_object({x:3, y:null}).product);
    });
  });



  describe('pass_array()', function () {
    /* pass_array accepts an array (input) and assumes it has 3 numeric elements at index 0-2.  
       It also looks for an additional property called "not_index".
       It contstructs a new array consisting of [input[0]+1, input.not_index, input[2]+1].
       No one said these functions should be sensible ;)
    */

    it('should fully compute given expected array', function () {
      var a = [4, 7, 9];
      a.not_index = "hello";
      assert.deepEqual([5, "hello", 10], loose.pass_array(a));
    });
    it('should return array with undefined values given incomplete input', function() {
      assert.deepEqual([2, undefined, 4], loose.pass_array([1, 2, 3]));
      assert.deepEqual([2, undefined, undefined], loose.pass_array([1, 2]));
      assert.deepEqual([undefined, undefined, undefined], loose.pass_array([]));
    });
  });
});


//---------------------------------------
// Strict module does exactly the same thing to input as loose example, but if there
// are ANY deviations from true data type, undefined is returned.  Check this module's code
// to see how error handling and strict type conversions can be handled in V8

describe('strict conversions', function() {


  describe('pass_number()', function () {
    it('should return input + 42 when given a valid number', function () {
      assert.equal(23+42, strict.pass_number(23));
      assert.equal(0.5+42, strict.pass_number(0.5));
    });

    it('should return undefined any time anything by the exact expected input is given', function () {
      assert.equal(undefined, strict.pass_number("23"));
      assert.equal(undefined, strict.pass_number("0.5"));
      assert.equal(undefined, strict.pass_number());
      assert.equal(undefined, strict.pass_number(null));
      assert.equal(undefined, strict.pass_number(undefined));
      assert.equal(undefined, strict.pass_number("this is not a number"));
      assert.equal(undefined, strict.pass_number({}));
      assert.equal(undefined, strict.pass_number({x: 5}));
      assert.equal(undefined, strict.pass_number([1, 2]));
    });
  });


  describe('pass_integer()', function () {
    it('should return input + 42 when given integer value', function () {
      assert.equal(5+42, strict.pass_integer(5));
    });
    it('should return undefined any time anything by the exact expected input is given', function () {
      assert.equal(undefined, strict.pass_integer(5.7));
      // pass_integer deals with non numbers the same as pass_number...
    });
  });


  describe('pass_string()', function () {
    
    it('should return reverse a proper string', function () {
      assert.equal(reverse("The truth is out there"), strict.pass_string("The truth is out there"));
    });
    
    it('should return undefined any time anything by the exact expected input is given', function () {
      assert.equal(undefined, strict.pass_string(42));
      assert.equal(undefined, strict.pass_string(true));
      assert.equal(undefined, strict.pass_string(null));
      assert.equal(undefined, strict.pass_string(undefined));
      assert.equal(undefined, strict.pass_string({x: 5}));
      assert.equal(undefined, strict.pass_string([9, 0]));
    });
  });


  describe('pass_boolean()', function () {
    
    
    it('should return compliment of proper boolean', function () {
      assert(strict.pass_boolean(true) == false);
      assert(strict.pass_boolean(false) == true);
    });

    it('should return undefined any time anything by the exact expected input is given', function () {
      assert.equal(undefined, strict.pass_boolean(42));
      assert.equal(undefined, strict.pass_boolean("is this true?"));
      assert.equal(undefined, strict.pass_boolean("true"));
      assert.equal(undefined, strict.pass_boolean({}));
      assert.equal(undefined, strict.pass_boolean({x: 5}));
      assert.equal(undefined, strict.pass_boolean([]));
      assert.equal(undefined, strict.pass_boolean([1, 2]));
      assert.equal(undefined, strict.pass_boolean());
      assert.equal(undefined, strict.pass_boolean(undefined));
      assert.equal(undefined, strict.pass_boolean(null));
    });


    
  });

  describe('pass_object()', function () {
    

    it('should fully compute properties given object with required properties', function () {
      assert.equal(13, strict.pass_object({x:3, y:10}).sum);
      assert.equal(30, strict.pass_object({x:3, y:10}).product);
    });
    it('should return undefined any time anything by the exact expected input is given', function () {
      assert.equal(undefined, strict.pass_object({y:10}));
      assert.equal(undefined, strict.pass_object({x:3}));
      assert.equal(undefined, strict.pass_object({x:3, y:"hello"}));
      assert.equal(undefined, strict.pass_object({x:3, y:true}));
      assert.equal(undefined, strict.pass_object({x:3, y:"10"}));
      assert.equal(undefined, strict.pass_object({x:3, y:null}));
    });
  });



  describe('pass_array()', function () {
    
    it('should fully compute given expected array', function () {
      var a = [4, 7, 9];
      a.not_index = "hello";
      assert.deepEqual([5, "hello", 10], strict.pass_array(a));
    });
    it('should return array with undefined values given incomplete input', function() {
      assert.equal(undefined, strict.pass_array([1, 2, 3]));
      assert.equal(undefined, strict.pass_array([1, 2]));
      assert.equal(undefined, strict.pass_array([]));
    });
  });
  
});




