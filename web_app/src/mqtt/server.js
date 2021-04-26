'use strict'

const debug = require('debug')("SG:MQTT server");
const chalk = require('chalk');
const mosca = require('mosca');
const DigitaldB = require('../models/Digital');
const AnalogdB = require('../models/Analog');
const Users = require('../models/User');
const {io, eventEmitter, devicesConnected,userConnected} = require('../utils/socketio.js'); 

//
// Mqtt nos permite la comunicacion entre el dispositivo y el servidor web.
//


const settings = {
	port: 1883
}
const server = new mosca.Server(settings)

let userTask = new Map();

// El servidor envia mensajes Mqtt al dispositivo
eventEmitter.on('action/setData', async function( data ){ 
  
  let i_socket = userConnected.get(data.MAC);

  console.log(`LLego al eventEmitter`);
  console.log(data);
  
  try {
    if(devicesConnected.has(data.MAC))
    {
      server.publish({
        topic: 'esp32/connect',
        payload: data.payload,
        qos: 0, // 0, 1, or 2
        retain: false // or true
      },i_socket.client, () => debug("Message sent"));
  
    }
    else
    {
      console.log(`ESP32 dont connected${data.MAC}`);
    }
  } catch (error) {
    debug( chalk.red('Error mqtt eventEmmiter ('+error+').') );
  }


});


/*
eventEmitter.on('user/update/water', async function(UserMac,plantIndex){  
  const index = parseInt(plantIndex,10);
  await Users.findOne( {MAC: UserMac }).then(doc => {

      let plant = doc.plants[index];
      plant.sowing.water.last_water = Date.now();
      doc.save();
      debug(chalk.green(`Update plant water ${plantIndex} in the user ${UserMac}`));
      userTask.delete(UserMac);
  });
});
*/
server.on('clientConnected', async client => {
  debug(`Client connected ${client.id}`);
});

server.on('clientDisconnected', client => {
  debug(`Client Disconnected ${client.id}`);
})

server.on('unsubscribed',(topic,client) => {
  debug(`Client unsubscribed ${client.id} with topic ${topic}`);
  
  let deviceId = (client.id).split('/')[1];


  if( topic == "esp32/connect")
  {
    // Eliminamos el dispositivo de la lista conectado.
    if ( devicesConnected.has(deviceId) )
    {
      let fl_status = devicesConnected.delete(deviceId);
      debug( chalk.yellow( `delete ESP32 connection MAC ${ deviceId } status ${ fl_status }`) );

    }
    
    if ( userConnected.has(deviceId) )
    {
      debug("Enviando MQTT al socket user offline");      
      userConnected.get(deviceId).socket.emit('action/user',"offline");
    }
  }

})
server.on('subscribed', (topic,client) => {
  let deviceId = (client.id).split('/')[1];

  debug(`Client subscribed ${deviceId} with topic ${topic}`);
  

  if( topic == "esp32/connect")
  {
    if ( !devicesConnected.has(deviceId) )
    {
      //Creo el usuario pero a la espera de asociarle un socket
      debug( chalk.yellow( `ESP32 connection MAC ${ deviceId }`) );
      devicesConnected.set(deviceId,{ online: true} );
    }
    
    // Comprobamos que el usuario esta conectado antes de enviar el mensaje
    if ( userConnected.has(deviceId) )
    {
      debug("Enviando MQTT al socket user online");    
      userConnected.get(deviceId).socket.emit('action/user',"online");
    }  
  }
  


  /*
  if( topic == "device/update/user"){
    if ( userTask.has(deviceId) ){
        debug( chalk.yellow( `searching update for ${deviceId}`) );
        userTask.get(deviceId).forEach(function(value, key, map){
          server.publish(value,client, () => debug("Message sent"));
        });
      userTask.delete(deviceId);
    }else{
      debug( chalk.yellow( `there are not updated for the user ${deviceId}`) );
      eventEmitter.emit('client/set/connection',deviceId );
    }
  }else{
    debug(chalk.yellow("It's don't the topic") );
  }
  */
})
server.on('published', async (packet, client) => {
  debug(`Received published with topic: ${packet.topic}`)

  /*
  if(packet.topic == "device/update/task"){
    let deviceMAC = (client.id).split('/')[1]; 
    let devicePayload = packet.payload.toString('utf-8').split('/');

    if (devicePayload[0] = "water"){
      eventEmitter.emit('user/update/water', deviceMAC, devicePayload[1] );
    }
  }
  */
  if(packet.topic == "Huerta/update/water"){
    debug( chalk.yellow('Msg water: '));
    debug( packet.payload.toString('utf-8') );

    let json_data = JSON.parse(packet.payload.toString('utf-8'));
    let fl_save = false;

    if(json_data != null){
      
      // offset de 2 horas y pasarlo de int a time_t
      json_data.timestamps = (json_data.timestamps + 2*60*60) *1000; 
      debug(chalk.yellow(`Update plant water ${Number(json_data.plant)} in the user ${json_data.device} with value ${json_data.status}`));

      await Users.findOne( {MAC: json_data.device }).then(doc => {

        //debug(chalk.yellow(`User found: `));
        for( const element in doc.plants){
          if (doc.plants[element].info.date == Number(json_data.plant) )
          {
            doc.plants[element].sowing.water.last_water =  Number(json_data.timestamps);
            debug(chalk.green(doc.plants[element].sowing.water ));
            fl_save = true;
          }
        }

        if(fl_save)
        {
          doc.save();
          debug(chalk.green("plant save"));
        }
      });
    }
  }
  if(packet.topic == "Huerta/update/light"){
    
    debug( chalk.yellow('Msg light: '));
    debug( packet.payload.toString('utf-8') );
    let fl_save = false;
    let json_data = JSON.parse(packet.payload.toString('utf-8'));
    
    if(json_data != null){
      
      // offset de 2 horas y pasarlo de int a time_t
      json_data.timestamps = (json_data.timestamps + 2*60*60) *1000; 
      debug(chalk.yellow(`Update plant light ${Number(json_data.plant)} in the user ${json_data.device} with value ${json_data.status}`));

      await Users.findOne( {MAC: json_data.device }).then(doc => {

        //debug(chalk.yellow(`User found: `));
        for( const element in doc.plants){
          if (doc.plants[element].info.date == Number(json_data.plant) ){
            
            if( Number(json_data.status) == 1){ // Si esta activo, guardamos la fecha acualizada
              doc.plants[element].sowing.light.last_light =  Number(json_data.timestamps);
              doc.plants[element].sowing.light.status = true;
            }
            else{
              doc.plants[element].sowing.light.status = false  ;
            }
            fl_save = true;
            debug(chalk.green(doc.plants[element].sowing.light ));
          }
        }
        if( fl_save)
        {
          doc.save();
          debug(chalk.green("plant save"));
        }
      });

    }

  }
  if (packet.topic == "Huerta/Push/Digital") {
    
    try {
      debug( chalk.yellow('Msg Dig: '));
      debug( packet.payload.toString('utf-8') );
      let json_data = JSON.parse(packet.payload.toString('utf-8'));
      if(json_data != null){

        json_data.timestamps = json_data.timestamps *1000;
        
        const newData = new DigitaldB(json_data);
        io.emit('chart/newData/digital', newData );
        
        await newData.save().then(()=>{
          debug('save :');
          debug(newData);
        }).catch((err)=>debug(`ERR`,err));    
      }
    }catch(err){
      debug( chalk.yellow('error de JSON in DigitaldB'))
      debug(err)
    }
  } 
  if (packet.topic == "Huerta/Push/Analog") {
    let json_data;
    let fl_save = false;

    try {
      //debug( chalk.yellow('Msg Anag: '));
      //debug( packet.payload.toString('utf-8') );
      json_data = JSON.parse(packet.payload.toString('utf-8'));
      if(json_data != null){

        // offset de 2 horas y pasarlo de int a time_t
        json_data.timestamps = (json_data.timestamps + 2*60*60) *1000; 
        
        const newData = new AnalogdB(json_data);
        io.emit('chart/newData/analog', newData );
        
        await newData.save().then(()=>{
          debug('save :');
          debug(newData);
        }).catch((err)=>debug(`ERR`,err));    
      }
    }catch(err){
      debug( chalk.yellow('error dee JSON'))
      debug(err)
    }

    debug( chalk.yellow('Msg last_value: '));
    
    if(json_data != null){

      await Users.findOne( {MAC: json_data.device }).then(doc => {

        //debug(chalk.yellow(`User found: `));
        for( const element in doc.plants){
          if (doc.plants[element].info.date == Number(json_data.plantId) ){
            
            doc.plants[element].info.analog.photocell1 =  json_data.photocell1.rawData;
            doc.plants[element].info.analog.photocell2 =  json_data.photocell1.rawData;
            doc.plants[element].info.analog.humEC      =  json_data.humEC.rawData;
            doc.plants[element].info.analog.humCap     =  json_data.humCap.rawData;
            doc.plants[element].info.analog.date       =  json_data.timestamps;

            fl_save = true;
            debug(chalk.yellow(doc.plants[element].info.analog));
          }
        }
        if( fl_save)
        {
          doc.save();
          debug(chalk.green("plant save"));
        }
      });

    }

  } 



  // Parte la comunicacion modo action
  if(packet.topic == "action/response"){
    let deviceMAC = (client.id).split('/')[1]; 
    let devicePayload = packet.payload.toString('utf-8').split('/');
    debug(chalk.green(`action/response" ${Number(devicePayload[0])} in the user ${deviceMAC} with value ${Number(devicePayload[1])}`));
    console.log(packet.payload.toString('utf-8'));

    debug("Enviando al socket");
    io.emit(`action/response/${deviceId}`, {text: packet.payload.toString('utf-8') });
  }



});

server.on('ready', async () => {
  debug( chalk.blue(` Server is running`));
})

server.on('error', handleFatalError)

module.exports = server


function handleFatalError (err) {
  debug( chalk.red(`[Fatal eror] ${err.message}`) );
  debug( chalk.red(err.stack));
  process.exit(1)
}

// Manejadores expeciones
process.on('uncaughtException', handleFatalError)
// Manejo de errores de las promesas
process.on('unhandledRejection', handleFatalError);
