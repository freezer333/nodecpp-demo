const loose = require('./loose/build/Release/loose_type_demo');

var assert = require('assert');
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
    function reverse(s) {
  		return s.split('').reverse().join('');
	}
  	it('should return reverse a proper string', function () {
      assert.equal(reverse("The truth is out there"), loose.pass_string("The truth is out there"));
    });
    it('should return reverse a numeric since numbers are turned into strings', function () {
      assert.equal("24", loose.pass_string(42));
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
});

