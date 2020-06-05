'use strict'
const mosca = require('mosca')
const DatadB = require('../models/DatadB');
const {io} = require('../utils/socketio.js');
const settings = {
	port: 1883
}
const server = new mosca.Server(settings)

let clientAux= {
  id: "",
  msg: ""
}

function getClienteAux( ){
  return clientAux;
}

function setClientAux( id, msg ){
  clientAux.id  = id;
  clientAux.msg = msg;
}
function clearClientAux(){
  clientAux.id  = "";
  clientAux.msg = "";
}

server.on('clientConnected', client => {
  console.log(`Client connected ${client.id}`);
})

server.on('clientDisconnected', client => {
  console.log(`Client Disconnected ${client.id}`);
})

server.on('subscribed', (topic,client) => {
  console.log(`Client ${client.id} with topic ${topic}`);
  server.publish({
    topic: 'esp32/MAC/system',
    payload: 'led/on/plan',
    qos: 0, // 0, 1, or 2
    retain: false // or true
  },client, () => console.log("Message sent LedON"));
  
  server.publish({
    topic: 'esp32/MAC/system',
    payload: 'led/off/pplan',
    qos: 0, // 0, 1, or 2
    retain: false // or true
  },client, () => console.log("Message sent LedOff")); 

  server.publish({
    topic: 'esp32/MAC/system',
    payload: 'water/on/plan',
    qos: 0, // 0, 1, or 2
    retain: false // or true
  },client, () => console.log("Message sent waterOn")); 

  server.publish({
    topic: 'esp32/MAC/system',
    payload: 'water/off/pplan',
    qos: 0, // 0, 1, or 2
    retain: false // or true
  },client, () => console.log("Message sent waterOff")); 
})
server.on('published', async (packet, client) => {
  
  if (packet.topic == "Huerta/Push/Digital"){ 
    setClientAux(client.id, packet.payload.toString('utf-8') )
    ;
    console.log(`Received: ${packet.topic}`) 
    console.log(`Client: `)
    console.log(getClienteAux())
  }

  if (packet.topic == "Huerta/Push/Analog") {
    let clientBef = getClienteAux();

    console.log(`Client: ${client.id}`)
    console.log(`before Client: ${clientBef.id}`)
    console.log(`Received: ${packet.topic}`)
    console.log(`Payload: `, packet.payload.toString('utf-8'));
    
    if( clientBef.id == client.id){
      try {
        let analog_json = JSON.parse(packet.payload.toString('utf-8'));
        let digital_json = JSON.parse(clientBef.msg);

        delete analog_json.timestamps;

        Object.keys(analog_json).forEach(function(key) { digital_json[key] = analog_json[key]; });


        if(digital_json != null){

          const newData = new DatadB(digital_json);
          io.emit('chart/NewData', newData );
          
          console.log(newData);
          await newData.save().then(()=>{
            console.log('save');
          }).catch((err)=>console.log(`ERR`,err));    
        }
      }catch(err){
        console.log('error dee JSON')
        console.log(err)
      }
    }
  }
});

server.on('ready', async () => {
  console.log(`[Smart-Garden-mqtt] server is running`);
})

server.on('error', handleFatalError)

module.exports = server


function handleFatalError (err) {
  console.error(`[Fatal eror] ${err.message}`)
  console.error(err.stack)
  process.exit(1)
}

// Manejadores expeciones
process.on('uncaughtException', handleFatalError)
// Manejo de errores de las promesas
process.on('unhandledRejection', handleFatalError)
