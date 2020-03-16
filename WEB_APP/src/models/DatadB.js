const mongoose = require('mongoose');
const { Schema }= mongoose;

const DataSchema = new Schema({
  
  Analog : {
    HumCap : {
      rawData : Number
    },
    HumEC : {
      rawData : Number
    },
    photocell1 : {
      Res : Number,
      rawData : Number
    },
    photocell2 : {
      Res : Number,
      rawData : Number
    }
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
  user:{ type: String, required:true},
  date:{ type: Date, default: Date.now }
})

module.exports = mongoose.model('Data', DataSchema)