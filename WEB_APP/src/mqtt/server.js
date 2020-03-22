'use strict'
const mosca = require('mosca')
const Data = require('../models/DatadB');

const settings = {
	port: 1883
}

const server = new mosca.Server(settings)

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
        const newData = new Data(json_data);
        await newData.save().then(()=>{
          console.log('save');
        }).catch((err)=>console.log(`ERR`,err));    
      }

    }catch(err){
      console.log('error dee JSON')
      
    }
  }
})

server.on('ready', async () => {
  console.log(`[Smart-Garden-mqtt] server is running`)
})

server.on('error', handleFatalError)

function handleFatalError (err) {
  console.error(`[Fatal eror] ${err.message}`)
  console.error(err.stack)
  process.exit(1)
}

// Manejadores expeciones
process.on('uncaughtException', handleFatalError)
// Manejo de errores de las promesas
process.on('unhandledRejection', handleFatalError)
