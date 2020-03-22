const mongoose = require('mongoose');
const { Schema }= mongoose;

const DataSchema = new Schema({
  
  HumCap : {
    rawData : Number
  },
  HumEC : {
    rawData : Number
  },
  Photocell1 : {
    Res : Number,
    rawData : Number
  },
  Photocell2 : {
    Res : Number,
    rawData : Number
  },
  BME280 : {
    Altitude : Number,
    Pressure : Number,
    Temp : Number
  },
  CCS811 : {
    CO2 : Number,
    TVOC : Number
  },
  Si7021 : {
    Humi : Number,
    Temp : Number
  },
  TCS34725 : {
    R : Number,
    G : Number,
    B : Number,
    C : Number,
    ColorTemp : Number,
    Lux : Number,
  },
  Device:{
    User:{ type: String, required:true},
    Device: { type: String, required:true}
  },
  Date:{ 
    Date: {type: String, required:true },
    Time: {type: String, required:true },
    timestamps: {type: Date, default: Date.now } 
   }
})

module.exports = mongoose.model('Data', DataSchema)