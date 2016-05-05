"use strict"; 

const emitStream = require('emit-stream');
const through = require('through');
const EventEmitter = require('events');

var make_stream = function(cpp_entry_point, opts) {
    const stream2node = require(cpp_entry_point);
    var emitter = new EventEmitter();

    var worker = new stream2node.StreamingWorker(
        function(event, value){
            emitter.emit(event, value);
        }, 
        function () {
            emitter.emit("close");
        }, 
        function(error) {
            emitter.emit("error", error);
        }, 
        opts);

    var sw = {};
    sw.fromAddon = emitter;
    sw.toAddon = {
        emit : function(name, data) {
            worker.sendToAddon(name, data);
        }
    }
    sw.close = function (){
        worker.closeInput();
    }

    sw.out = emitStream(sw.fromAddon).pipe(through(function (data) {
                    if ( data[0] == "close"){
                        this.end();
                    }
                    else {
                        this.queue(data);
                    }

                }));

    

    sw.build_input_stream = function(end) {
        var input = through(function write(data) {
            if ( data[0] == "close"){
                this.end();
            }
            else {
                sw.toAddon.emit(data[0], data[1]);
            }
          },
          end);
        return input;
    }

    return sw;
}

module.exports = make_stream;
