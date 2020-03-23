'use strict'
const mosca = require('mosca')
const DatadB = require('../models/DatadB');

const settings = {
	port: 1883
}
const server = new mosca.Server(settings)

class SmartGardenAgent {

  constructor(io){
      this._io = io;
  }

  connectMqttServer(){
    server.on('clientConnected', client => {
      console.log(`Client connected ${client.id}`)
    })
    
    server.on('clientDisconnected', client => {
      console.log(`Client Disconnected ${client.id}`)
    })
    
    server.on('published', async (packet, client) => {
      console.log(`Received: ${packet.topic}`)
      console.log(`Payload: `);
      console.log(packet.payload.toString('utf-8'));
      
      if ((packet.topic == "Huerta/Push/Digital") || (packet.topic == "Huerta/Push/Analog")) {
        
        try {
          let json_data = JSON.parse(packet.payload.toString('utf-8'));
          if(json_data != null){
            json_data["Date"] = 
            { Date: (new Date).toISOString().substr(0,10),
              Time: (new Date).toISOString().substr(11,8)
            };
            
            console.log(json_data);
            const newData = new DatadB(json_data);
            this._io.emit('chart/NewData', newData );
            await newData.save().then(()=>{
              console.log('save');
            }).catch((err)=>console.log(`ERR`,err));    
          }
        }catch(err){
          console.log('error dee JSON')
          console.log(err)
        }
      }
    })
    
    server.on('ready', async () => {
      console.log(`[Smart-Garden-mqtt] server is running`)
    })
    
    server.on('error', handleFatalError)
  }
  connectSocketio(){
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
                  socket.emit('chart/Err', {text: "Sin Datos", Data} ); 
              }
  
          }).catch( (error )=>{
              console.log("Error database mongo:");
              console.log(error);
              socket.emit('chart/Err',{text: "Error dB", err: error});
          });
      });  
    });
  }

  disconnect(){

  }
}
module.exports = SmartGardenAgent


function handleFatalError (err) {
  console.error(`[Fatal eror] ${err.message}`)
  console.error(err.stack)
  process.exit(1)
}

// Manejadores expeciones
process.on('uncaughtException', handleFatalError)
// Manejo de errores de las promesas
process.on('unhandledRejection', handleFatalError)
