"use strict";

// Obtenemos valores de los dias que quiere visualizar los datos
let user_date = document.getElementById("user_date");
let chart_lengend = document.getElementById("chart_lengend");
user_date.valueAsDate = new Date(); //La primera vez que carga la pagina, actualizamos la fecha

// Luego utilizar esta variable con el login del usuario
let user_name = "Usuario1";

let progress = document.getElementById('animationProgress');
let config = {
  "type":"line",
  "data":{
    "labels":["00:00","01:00","02:00","03:00","04:00","05:00"],
    "datasets":[]
  },
  options:{
    deviceSmall: false,
    events: ['click'],
    responsive: true,
    legend: {display: true},
    title: {
      display: true,
      text: 'Muestra diarias'
    },
    tooltips: {
      mode: 'index',
      intersect: false,
    },
    hover: {
      mode: 'nearest',
      intersect: true
    },
    animation: {
      duration: 2000,
      onProgress: function(animation) {
        progress.value = animation.currentStep / animation.numSteps;
      },
      onComplete: function() {
        window.setTimeout(function() {
          progress.value = 0;
        }, 2000);
      }
    },
    scales: {
      xAxes: [{
        display: true,
        scaleLabel: {
          display: true,
          labelString: 'Hora'
        }
      }],
      yAxes: [{
        display: true,
        scaleLabel: {
          display: true,
          labelString: 'Valor'
        }
      }]
      }
  }
};
let LocalDatabase = []; // Almacenamos las metricas temporalmente
let socket; // Aun no se inicializa el socket.io

// Once the page load is finished and user logs in, the grafics are inialized
window.onload = function() {
  let ctx = document.getElementById('myChart').getContext('2d');
  window.myLineChart = new Chart(ctx, config);

  socket = io(); //Inicializamos los sockets
  UpdateDate();  //Peticion de las metricas almacenadas en un dia en especifico

  if (document.documentElement.clientWidth < 800){
    config.options.legend.display = false;
    config.options.scales.xAxes = false;
    config.options.scales.yAxes = false;
    config.deviceSmall = true;
  }

  // Socket que escucha las peticiones de recepcion de datos
  socket.on('chart/PostData', (Data)=>{
    console.log(socket.id);
    console.log('Recibido al cliente');
    console.log(Data);
    LocalDatabase= Data;
    updateData();
  });

  socket.on('chart/NewData', (Data)=>{
    console.log(socket.id);
    console.log('Nueva Data disponible al cliente');
    console.log(Data);
    LocalDatabase.push(Data);
    updateData();
  });

  // Socket que escucha los posibles errores 
  socket.on('chart/Err', (Data)=>{
    console.log(Data);
    switch(Data.text){
      case "Sin Datos": console.log(`Para el Usuario ${Data.Data.User} en el dia ${Data.Data.Date}, No hay datos`);
      break;
      case "Error dB": console.log(`Error en la base de datos`);console.log(Data.err);
      break
      default: console.log(Data);
    }
    clearData();
  });
  
};


function updateData(){
    config.data.labels = [];
    for (const prop in LocalDatabase){
      config.data.labels.push(LocalDatabase[prop].Date.Time);
    }

    let old_dataset = config.data.datasets;
    config.data.datasets = [];

    for(const dataset_name in old_dataset){
      old_dataset[dataset_name].func();
    }
    
    updateChart();
    
    console.log("actualizada la base de datos local");
    
}
function addData_database(){
  let data = {
    hora: "00:00",
    lumenes: 112,
    Temp: 122,
    Red: 22,
    Green: 0,
    Blue: 0
  };
  Hora_diaria.forEach( (element, indice )=> {
    data.hora = element;
    data.lumenes  = data_lumenes[indice];
    data.Temp = data_temperatura[indice];
    data.Red = data_rojo[indice];
    data.Blue = data_azul[indice];
    data.Green = data_verde[indice];
    saveContactForm(1,data);
    
  });
}

function updateChart(){
  if (config.deviceSmall ){
    chart_lengend.innerHTML = window.myLineChart.generateLegend();
  }
  window.myLineChart.update();
}
// Delete the data that could user add
function clearData(){
  config.data.datasets = [];
  //config.data.labels =["00:00","01:00","02:00","03:00","04:00","05:00"];
  updateChart()
}
function UpdateDate(){
  console.log(`the user: ${user_name} on the day :${user_date.value} `); 
  socket.emit('chart/getData', {User: user_name, Date: user_date.value });
  LocalDatabase = {};
}

// Metrics representation of the analogs sensors
function Analog_humec() {
  let newDataset = {
    label: 'HumEC',
    func: Analog_humec,
    backgroundColor: "rgb(12, 1, 90)",
    borderColor: "rgb(12, 1, 90)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].HumCap){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time, y:LocalDatabase[prop].HumEC.RawData});
      }
    }
    config.data.datasets.push(newDataset);
    updateChart();
  }
}
function Analog_humcap() {
  let newDataset = {
    label: 'HumCap',
    func: Analog_humcap,
    backgroundColor: "rgb(12, 1, 20)",
    borderColor: "rgb(12, 1, 20)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].HumCap){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time, y:LocalDatabase[prop].HumCap.RawData});
      }
    }
    config.data.datasets.push(newDataset);
    updateChart();
  }
}
function Analog_photocell2() {
  var newDataset = {
    label: 'photocell2',
    func: Analog_photocell2,
    backgroundColor: "rgb(12, 1, 20)",
    borderColor: "rgb(12, 1, 20)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].Photocell2){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time, y:LocalDatabase[prop].Photocell2.RawData});
      }
    }
    config.data.datasets.push(newDataset);
    updateChart();
  }
}
function Analog_photocell1() {
  var newDataset = {
    label: 'photocell1',
    func:Analog_photocell1,
    backgroundColor: "rgb(123, 12, 20)",
    borderColor: "rgb(123, 12, 20)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].Photocell1){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time, y:LocalDatabase[prop].Photocell1.RawData});
      }
    }
    config.data.datasets.push(newDataset);
    updateChart();
  }
}
// Metrics representation of the BME280 sensor
function BME280_alt() {
  var newDataset = {
    label: 'Altitude',
    func:BME280_alt,
    backgroundColor: "rgb(13, 12, 200)",
    borderColor: "rgb(13, 12, 200)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].BME280){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time , y:LocalDatabase[prop].BME280.Altitude});
      }
    }
    config.data.datasets.push(newDataset);
    updateChart();
  }
}
function BME280_pre() {
  var newDataset = {
    label: 'Pressure',
    func:BME280_pre,
    backgroundColor: "rgb(13, 212, 0)",
    borderColor: "rgb(13, 212, 0)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].BME280){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time , y:LocalDatabase[prop].BME280.Pressure});
      }
    }
    config.data.datasets.push(newDataset);
    updateChart();
  }
}
function BME280_temp() {
  var newDataset = {
    label: 'Temp_BME',
    func: BME280_temp,
    backgroundColor: "rgb(131, 212, 0)",
    borderColor: "rgb(131, 212, 0)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].BME280){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time , y:LocalDatabase[prop].BME280.Temp});
      }
    }
    config.data.datasets.push(newDataset);
    updateChart();
  }
}
// Metrics representation of the CCS811 sensor
function CCS811_TVOC() {
  let newDataset = {
    label: 'TVOC',
    func: CCS811_TVOC,
    backgroundColor: "rgb(13, 12, 87)",
    borderColor: "rgb(13, 12, 87)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].CCS811){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time , y:LocalDatabase[prop].CCS811.TVOC});
      }
    }
    config.data.datasets.push(newDataset);
    updateChart();
  }
}
function CCS811_CO2() {
  let newDataset = {
    label: 'CO2',
    func: CCS811_CO2,
    backgroundColor: "rgb(43, 152, 87)",
    borderColor: "rgb(43, 152, 87)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].CCS811){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time , y:LocalDatabase[prop].CCS811.CO2});
      }
    }
    config.data.datasets.push(newDataset);
    updateChart();
  }
}
// Metrics representation of the Si7021 sensor
function Si7021_hum() {
  var newDataset = {
    label: 'Hum_Si7',
    func: Si7021_hum,
    backgroundColor: "rgb(13, 152, 87)",
    borderColor: "rgb(13, 152, 87)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].Si7021){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time , y:LocalDatabase[prop].Si7021.Humi});
      }
    }
    config.data.datasets.push(newDataset);
    updateChart();
  }
}
function Si7021_temp() {
  var newDataset = {
    label: 'Temp_Si7',
    func: Si7021_temp,
    backgroundColor: "rgb(138, 152, 87)",
    borderColor: "rgb(138, 152, 87)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].Si7021){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time , y:LocalDatabase[prop].Si7021.Temp });
      }
    }
    config.data.datasets.push(newDataset);
    updateChart();
  }
}
// Metrics representation of the TCS34725 sensor
function TCS34725_c() {
  let newDataset = {
    label: 'Clear_TCS',
    func:TCS34725_c,
    backgroundColor: "rgb(251, 201, 0)",
    borderColor: "rgb(251,201, 0)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
      for (const prop in LocalDatabase){
        if(LocalDatabase[prop].TCS34725){
          newDataset.data.push({x:LocalDatabase[prop].Date.Time , y:LocalDatabase[prop].TCS34725.C } );
        }
      }
      config.data.datasets.push(newDataset);
    };
    updateChart();
}
function TCS34725_lum() {
  let newDataset = {
    label: 'Lux_TCS',
    func:TCS34725_lum,
    backgroundColor: "rgb(25, 20, 0)",
    borderColor: "rgb(25,20, 0)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
      for (const prop in LocalDatabase){
        if(LocalDatabase[prop].TCS34725){
          newDataset.data.push({x:LocalDatabase[prop].Date.Time , y:LocalDatabase[prop].TCS34725.Lux } );
        }
      }
      config.data.datasets.push(newDataset);
    };
    updateChart();
}
function TCS34725_r() {
  let newDataset = {
    label: 'Luz_Roja',
    func:TCS34725_r,
    backgroundColor: "rgb(255, 0, 0)",
    borderColor: "rgb(255, 0, 0)",
    data: [],
    fill: false
  };

  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
      for (const prop in LocalDatabase){
        if(LocalDatabase[prop].TCS34725){
          newDataset.data.push({x:LocalDatabase[prop].Date.Time, y:LocalDatabase[prop].TCS34725.R } );
        }
      }
      config.data.datasets.push(newDataset);
    };
}
function TCS34725_g() {
  if (config.data.datasets.find( element =>element.label === 'Luz_Verde') ){
    console.log ( "Ya existe esta grafica");
  }else{
      var newDataset = {
        label: 'Luz_Verde',
        func:TCS34725_g,
        backgroundColor: "rgb(0, 255, 0)",
        borderColor: "rgb(0, 255, 0)",
        data: [],
        fill: false
      };
      for (const prop in LocalDatabase){
        if(LocalDatabase[prop].TCS34725){
          newDataset.data.push({x:LocalDatabase[prop].Date.Time, y:LocalDatabase[prop].TCS34725.G } );
        }
      }
      config.data.datasets.push(newDataset);
    };
}
function TCS34725_b() {
  let newDataset = {
    label: 'Luz_Azul',
    func:TCS34725_b,
    backgroundColor: "rgb(0, 0, 255)",
    borderColor: "rgb(0, 0, 255)",
    data: [],
    fill: false
  };
  if (config.data.datasets.find( element =>element.label === newDataset.label) ){
    console.log ( "Ya existe esta grafica");
  }else{
    for (const prop in LocalDatabase){
      if(LocalDatabase[prop].TCS34725){
        newDataset.data.push({x:LocalDatabase[prop].Date.Time , y:LocalDatabase[prop].TCS34725.B });
      }
    }
    config.data.datasets.push(newDataset);
  };
  updateChart();
}
function TCS34725_rgb() {
  TCS34725_r();
  TCS34725_g();
  TCS34725_b();
}


// Add buttom event listener
document.getElementById("TCS34725_rgb").addEventListener('click', TCS34725_rgb);
document.getElementById('TCS34725_lum').addEventListener('click', TCS34725_lum );
document.getElementById('TCS34725_c').addEventListener('click', TCS34725_c );

document.getElementById('Si7021_temp').addEventListener('click', Si7021_temp );
document.getElementById('Si7021_hum').addEventListener('click', Si7021_hum );

document.getElementById('CCS811_CO2').addEventListener('click', CCS811_CO2 );
document.getElementById('CCS811_TVOC').addEventListener('click', CCS811_TVOC );

document.getElementById('BME280_alt').addEventListener('click', BME280_alt );
document.getElementById('BME280_pre').addEventListener('click', BME280_pre );
document.getElementById('BME280_temp').addEventListener('click', BME280_temp );

document.getElementById('Analog_photocell1').addEventListener('click', Analog_photocell1 );
document.getElementById('Analog_photocell2').addEventListener('click', Analog_photocell2 );
document.getElementById('Analog_humcap').addEventListener('click', Analog_humcap );
document.getElementById('Analog_humec').addEventListener('click', Analog_humec );

document.getElementById('clearData').addEventListener('click', clearData );
document.getElementById('updateData').addEventListener('click', UpdateDate );


  
