'use strict'
const AnalogdB = require('../models/Analog');
const DigitaldB = require('../models/Digital');
const {io} = require('../utils/socketio.js');
io.on('connection',   (socket)=>{
  console.log('[Smart-Garden-Socket] connected '+ socket.id)
  
  socket.on('chart/getData/digital', (Data)=>{
    console.log('Recibido al Servidor: ', Data);
    console.log('Del Usuario: ', socket.id);
    
    DigitaldB.find({ 
          "device": Data.Device,
          "timestamps": {     
            $gte: (new Date(`${Data.Date}T00:00:00.000Z`)),     
            $lt:  (new Date(`${Data.Date}T23:59:59.189Z`))
          }
        }).sort({timestamps: 1})
        .then( dataUser =>{
          console.log("We find this: ", dataUser.length);
          if(dataUser.length != 0){
              socket.emit('chart/postData/digital', dataUser );  
          }else{
              socket.emit('chart/err', {text: "empty Data to digital", Data} ); 
          }
        }).catch( (error )=>{
            console.log("Error database digitalmongo:");
            console.log(error);
            socket.emit('chart/err',{text: "Error dB digital", err: error});
        });
  }); 
  
  socket.on('chart/getData/analog', (Data)=>{
    console.log('Recibido al Servidor: ', Data);
    console.log('Del Usuario: ', socket.id);
    
    AnalogdB.find({ 
          "device": Data.Device,
          "timestamps": {     
            $gte: (new Date(`${Data.Date}T00:00:00.000Z`)),     
            $lt:  (new Date(`${Data.Date}T23:59:59.189Z`))
          }
        }).sort({timestamps: 1})
        .then( dataUser =>{
          console.log("We find this: ", dataUser.length);
          if(dataUser.length != 0){
              socket.emit('chart/postData/analog', dataUser );  
          }else{
              socket.emit('chart/err', {text: "empty Data to analog", Data} ); 
          }
        }).catch( (error )=>{
            console.log("Error database analog mongo:");
            console.log(error);
            socket.emit('chart/err',{text: "Error dB analog", err: error});
        });
  });  
});
console.log(`[Smart-Garden-Socketio]: connected`);