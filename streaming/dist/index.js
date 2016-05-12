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
    

    sw.from = emitter;
    sw.from.stream = function() {
        return emitStream(sw.from).pipe(
            through(function (data) {
                if ( data[0] == "close"){
                    this.end();
                }
                else {
                    this.queue(data);
                }
            }));
    }

    sw.to = {
        emit : function(name, data) {
            worker.sendToAddon(name, data);
        },
        stream : function(name, end) {
            var input = through(function write(data) {
                if (Array.isArray(data)) {
                    if ( data[0] == "close"){
                        this.end();
                    }
                    else {
                        sw.to.emit(data[0], data[1]);
                    }
                }
                else {
                    sw.to.emit(name, data);
                }
          },
          end);
        return input;
        }
    }
    sw.close = function (){
        worker.closeInput();
    }

   
    return sw;
}

module.exports = make_stream;
