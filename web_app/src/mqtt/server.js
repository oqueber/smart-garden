'use strict'

const debug = require('debug')("SG:MQTT server");
const chalk = require('chalk');
const mosca = require('mosca');
const DigitaldB = require('../models/Digital');
const AnalogdB = require('../models/Analog');
const Users = require('../models/User');
const {io, eventEmitter, devicesConnected,userConnected} = require('../utils/socketio.js'); 

const settings = {
	port: 1883
}
const server = new mosca.Server(settings)

let userTask = new Map();

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
eventEmitter.on('client/set/connection', async function( id ){
  if ( !userTask.has( id ) ){
    let task = new Promise( async (resolve, reject) => {
      const user = await Users.findOne({MAC: id});
       
      if(user){
        if(user.plants.length >= 1){
          let task_for_plant = new Map();
          
          user.plants.forEach( (element, index) => {
            let today = Date.now();
            let last_water     = element.sowing.water.last_water;
            let frequency_water= element.sowing.water.frequency;
            let limit_water = element.sowing.water.limit;
            let last_light = element.sowing.light.last_light;
            debug( chalk.blue( ` ${(today-last_water)/(1000*60*60*24)} > ${frequency_water}`) );
            if( ((today-last_water)/(1000*60*60*24)) >  frequency_water){
              task_for_plant.set(index,{
                topic: 'device/get/task',
                payload: 'water/on/'+index+"/"+limit_water,
                qos: 0, // 0, 1, or 2
                retain: false // or true
              });
            }
          }); 
          
          if( task_for_plant.size > 0){
            resolve(task_for_plant);
          }else{
            reject(Error("Nothing to doing right now"))
          }
        }else{
          reject(Error("The user don't have plants"));
        }
      }else{
        reject(Error("User don't found"));
      }

    });
    task.then( function(tasks) {
      userTask.set(id,tasks);

    }).catch(function(reason) {
      debug( chalk.yellow('Manejar promesa rechazada ('+reason+') aquí.') );
    });
    
  }else{
    //already have a notice
    debug('the user already have a task');
  }
});
*/
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
    let deviceMAC = (client.id).split('/')[1]; 
    let devicePayload = packet.payload.toString('utf-8').split('/');
    debug(chalk.green(`Update plant water ${Number(devicePayload[0])} in the user ${deviceMAC} with value ${Number(devicePayload[1])}`));
    console.log(packet.payload.toString('utf-8'));
    await Users.findOne( {MAC: deviceMAC }).then(doc => {

      //debug(chalk.yellow(`User found: `));
      for( const element in doc.plants){

        if (doc.plants[element].info.date == Number(devicePayload[0]) ){
          doc.plants[element].sowing.water.last_water = Number(devicePayload[1]) ;
          debug(chalk.green(doc.plants[element].sowing.water ));
        }

      }
      doc.save();
      debug(chalk.green("plant save"));
      //debug(chalk.green(doc.plants ));
    });
  }
  if(packet.topic == "Huerta/update/light"){
    let deviceMAC = (client.id).split('/')[1]; 
    let devicePayload = packet.payload.toString('utf-8').split('/');
    debug(chalk.yellow(`Update plant light ${Number(devicePayload[0])} in the user ${deviceMAC} with value ${Number(devicePayload[1])}`));
    console.log(packet.payload.toString('utf-8'));
    await Users.findOne( {MAC: deviceMAC }).then(doc => {

      //debug(chalk.yellow(`User found: `));
      for( const element in doc.plants){
        if (doc.plants[element].info.date == Number(devicePayload[0]) ){
          
          if( Number(devicePayload[1]) == 1){ // Si esta activo, guardamos la fecha acualizada
            doc.plants[element].sowing.light.last_light =  Number(devicePayload[2])  ;
            doc.plants[element].sowing.light.status = true;
          }
          else{
            doc.plants[element].sowing.light.status = false  ;
          }
          
          debug(chalk.green(doc.plants[element].sowing.light ));
        }
      }
      doc.save();
      debug(chalk.green("plant save"));
    });
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
    
    try {
      //debug( chalk.yellow('Msg Anag: '));
      //debug( packet.payload.toString('utf-8') );
      let json_data = JSON.parse(packet.payload.toString('utf-8'));
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
