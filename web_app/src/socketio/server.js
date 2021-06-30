'use strict'
const AnalogdB = require('../models/Analog');
const DigitaldB = require('../models/Digital');
const debug = require('debug')("SG:socketio ");
const chalk = require('chalk');
const {io, eventEmitter, devicesConnected,userConnected} = require('../utils/socketio.js');


//
// SocketIo nos permite realiar comunicacion entre la plataforma web y el servidor.
//

io.on('connection',   (socket)=>{
  debug( chalk.yellow( `Connected  ${socket.id }` ));
  
  // Usuario conectado a la plataforma web
  socket.on('user/connect', (data)=>{

    // Comprobar si el usuario ya está conectado
    if ( !userConnected.has(data.MAC))
    {
      let fl_status =  userConnected.set( data.MAC , { socket: socket, socketId:socket.id} );
      debug( chalk.yellow( `new web connection with ${socket.id} and MAC ${data.MAC} status ${fl_status}`)); 
    }
    else
    {
      // Actualizamos el socket por nueva conexion.
      debug(chalk.yellow( `already exits connection with MAC: ${data.MAC}`));        
      userConnected.get(data.MAC).socket = socket;
      userConnected.get(data.MAC).socketId = socket.id;
    }

    // Comprobar si el dispositivo ya esta conectado al servidor,
    if ( devicesConnected.has(data.MAC) )
    {
      debug(`The garden its already online ${data.MAC}`); 
      socket.emit('action/user',"online");
    }

  }); 

  // Usuario cse desconecto de la plataforma web
  socket.once('disconnect', function () {

    for(let [i_MAC, i_value]  of userConnected )
    {
      if ( socket.id == i_value.socketId )
      {
        if( i_value.esp32 == false )
        {
          let fl_status = userConnected.delete(i_MAC);
          debug( chalk.yellow( `delete web connection ${i_value.socketId} and MAC ${ i_MAC } status ${ fl_status }`) );
        }
      }
    }
  });

  // Cuando el usuario y el dispositivo estan conectado a la plataforma web
  socket.on('action/setData', (Data)=>{
    debug('Recibido al Servidor: ', Data);
    debug('Del Usuario: ', socket.id);
        
    if ( devicesConnected.has(Data.MAC) )
    {
      debug("enviando al eventEmitter ");
      eventEmitter.emit('action/setData',{ MAC: Data.MAC, payload: JSON.stringify(Data.payload) } );
    }
    else
    {
      socket.emit('action/response',{text: "Huerto no conectado"});
    }

  }); 

  socket.on('chart/getData/digital', (Data)=>{
    //console.log('Recibido al Servidor: ', Data);
    //console.log('Del Usuario: ', socket.id);
    
    DigitaldB.find({ 
          "device": Data.Device,
          "timestamps": {     
            $gte: (new Date(`${Data.Date}T00:00:00.000Z`)),     
            $lt:  (new Date(`${Data.Date}T23:59:59.189Z`))
          }
        }).sort({timestamps: 1})
        .then( dataUser =>{
          //console.log("We find this: ", dataUser.length);
          if(dataUser.length != 0){
              socket.emit('chart/postData/digital', dataUser );  
          }else{
              socket.emit('chart/err', {text: "No hay datos digitales", Data} ); 
          }
        }).catch( (error )=>{
            console.log("Error database digitalmongo:");
            console.log(error);
            socket.emit('chart/err',{text: "Error dB digital", err: error});
        });
  }); 
  
  socket.on('chart/getData/analog', (Data)=>{
    //console.log('Recibido al Servidor analog: ', Data);
    //console.log('Del Usuario: ', socket.id);
    
    AnalogdB.find({ 
          "device": Data.Device,
          "timestamps": {     
            $gte: (new Date(`${Data.Date}T00:00:00.000Z`)),     
            $lt:  (new Date(`${Data.Date}T23:59:59.189Z`))
          }
        }).sort({timestamps: 1})
        .then( dataUser =>{
          let plant_select;
          if ( Data.Plant ){
            plant_select = Data.Plant;
          }else{
            plant_select = 0
          }
          debug("Datos almacenados del usuario: ", dataUser.length);
          if(dataUser.length != 0){ 
            //console.log(dataUser);
            //console.log("plant_select: ", plant_select);
            
            dataUser = dataUser.filter(function (a) {
              return a.plantId === plant_select;
            });
            //console.log(dataUser);
            if(dataUser.length != 0){ 
              socket.emit('chart/postData/analog', dataUser );  
            }else {
              socket.emit('chart/err', {text: "No hay datos analogicos", Data} );
            }
          }else{
              socket.emit('chart/err', {text: "No hay datos analogicos", Data} ); 
          }
        }).catch( (error )=>{
            console.log("Error database analog mongo:");
            console.log(error);
            socket.emit('chart/fatal/err',{text: "Error dB analog", err: error});
        });
  });  

});

debug( chalk.blue(` SocketIO is running`));
