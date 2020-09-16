const mongoose = require('mongoose');
const { Schema }= mongoose;

const DataSchema = new Schema({
  
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
  type: { type: String, default:"Digital"},
  device: { type: String, required:true},
  plantId: { type: String},
  timestamps: {type: Number } 
   
})

module.exports = mongoose.model('Digital', DataSchema)