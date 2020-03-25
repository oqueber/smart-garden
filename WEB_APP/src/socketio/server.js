'use strict'
const DatadB = require('../models/DatadB');

class SmartGardenIo {

  constructor(io){
      this._io = io;
  }
  connect(){
    this._io.on('connection',   (socket)=>{
      console.log('[Smart-Garden-Socket] connected'+ socket.id)
      
      socket.on('chart/getData', (Data)=>{
          console.log(socket.id);
          console.log('Recibido al cliente');
          console.log(Data);
        
          DatadB.find({ 
              "Device.User": Data.User,
              "Date.Date": Data.Date
          }).then( dataUser =>{
              if(dataUser.length != 0){
                  socket.emit('chart/PostData', dataUser );  
              }else{
                  socket.emit('chart/Err', {text: "No hay datos", Data} ); 
              }
  
          }).catch( (error )=>{
              console.log("Error database mongo:");
              console.log(error);
              socket.emit('chart/Err',{text: "Error dB", err: error});
          });
      });  
    });
  }
}
module.exports = SmartGardenIo
