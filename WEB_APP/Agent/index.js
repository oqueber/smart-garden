'use strict'

const EventEmitter = require('events');

class SmartGardenAgent extends EventEmitter{

    constructor(opts){
        super()
        this._options = opts;
        this._started = false;
        this._timer = null;
    }

    connect(){
        if(!this._started){
            this._started = true;
            this.emit('connect');
            const opts = this._options;
            this._timer = setInterval(()=>{
                this.emit('agent/message', 'this is a message')
            },opts.interval)
        }
    }

    disconnect(){
        if(this._started){
            clearInterval(this._timer);
            this._started = false;
            this.emit('disconnect');
        }
    }
}
module.exports = SmartGardenAgent