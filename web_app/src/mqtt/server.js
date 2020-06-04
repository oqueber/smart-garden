'use strict'
const mosca = require('mosca')
const DatadB = require('../models/DatadB');
const {io} = require('../utils/socketio.js');
var interval1_on, interval1_off, interval2_on,interval2_off;
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
function sendMqtt1_on(){
  console.log("enviando 1 on");
  server.publish({
    topic: 'esp32/MAC/system',
    payload: 'led/on/plan'
  });
}
function sendMqtt1_off(){
  console.log("enviando 1 off");
  server.publish({
    topic: 'esp32/MAC/system',
    payload: 'led/off/plan'
  });
}
function sendMqtt2_on(){
  console.log("enviando 2 on");
  server.publish({
    topic: 'esp32/MAC/system',
    payload: 'water/on/plan'
  });
}
function sendMqtt2_off(){
  console.log("enviando 2 off");
  server.publish({
    topic: 'esp32/MAC/system',
    payload: 'water/off/plan'
  });
}
server.on('clientConnected', client => {
  console.log(`Client connected ${client.id}`);  
})

server.on('clientDisconnected', client => {
  console.log(`Client Disconnected ${client.id}`)
  //clearInterval(interval1_on);
  //clearInterval(interval1_off);
  //clearInterval(interval2_on);
  //clearInterval(interval2_off);
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
})




server.on('ready', async () => {
  console.log(`[Smart-Garden-mqtt] server is running`);
  interval1_on  = setInterval(sendMqtt1_on, 2000);
  interval1_off = setInterval(sendMqtt1_off,2500);
  interval2_on  = setInterval(sendMqtt2_on, 3000);
  interval2_off = setInterval(sendMqtt2_off,3500);
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
