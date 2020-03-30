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
        console.log(new Date(`${Data.Date}T00:00:00.000Z`),);
        console.log('al');
        console.log(new Date(`${Data.Date}T23:59:59.189Z`));
        
        DatadB.find({ 
              //"Device": Data.Device,
              "timestamps": {     
                $gte: (new Date(`${Data.Date}T00:00:00.000Z`)),     
                $lt:  (new Date(`${Data.Date}T23:59:59.189Z`))
              }
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
