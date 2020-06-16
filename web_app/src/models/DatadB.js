const mongoose = require('mongoose');
const { Schema }= mongoose;

const DataSchema = new Schema({
  
  humCap : {
    rawData : Number
  },
  humEC : {
    rawData : Number
  },
  photocell1 : {
    rawData : Number
  },
  photocell2 : {
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
  device: { type: String, required:true},
  plantId: { type: String},
  timestamps: {type: Date, default: Date.now } 
   
})

module.exports = mongoose.model('Data', DataSchema)