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
  type: { type: String, default:"Analog"},
  device: { type: String, required:true},
  plantId: { type: String},
  timestamps: {type: Date } 
   
})

module.exports = mongoose.model('Analog', DataSchema)