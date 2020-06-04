'use strict'
const DatadB = require('../models/DatadB');
const {io} = require('../utils/socketio.js');
io.on('connection',   (socket)=>{
  console.log('[Smart-Garden-Socket] connected'+ socket.id)
  
  socket.on('chart/getData', (Data)=>{
    console.log('Recibido al Servidor: ', Data);
    console.log('Del Usuario: ', socket.id);
    
    DatadB.find({ 
          "Device": Data.Device,
          "timestamps": {     
            $gte: (new Date(`${Data.Date}T00:00:00.000Z`)),     
            $lt:  (new Date(`${Data.Date}T23:59:59.189Z`))
          }
        }).sort({timestamps: 1})
        .then( dataUser =>{
          console.log("We find this: ", dataUser.length);
          if(dataUser.length != 0){
              socket.emit('chart/PostData', dataUser );  
          }else{
              socket.emit('chart/Err', {text: "empty Data", Data} ); 
          }
        }).catch( (error )=>{
            console.log("Error database mongo:");
            console.log(error);
            socket.emit('chart/Err',{text: "Error dB", err: error});
        });
  });  
});
console.log(`[Smart-Garden-Socketio]: connected`);