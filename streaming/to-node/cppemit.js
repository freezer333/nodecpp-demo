const stream2node = require('./build/Release/stream_to_node');
const EventEmitter = require('events');

var make_stream = function() {
    const emitter = new EventEmitter();
    stream2node.start(1, function(event, value){
        emitter.emit(event, value);
    }, function () {
        emitter.emit("close");
    });

    return emitter;
}

module.exports = make_stream;
