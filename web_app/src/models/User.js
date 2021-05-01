const mongoose = require('mongoose');
const { Schema }= mongoose;
const bcrypt = require("bcryptjs");

const UserSchema = new Schema({
    name: { type: String, required:true },
    email: { type: String, required:true },
    password: { type: String, required:true },
    zip: { type: Number},
    state: { type: String},
    city: { type: String},
    MAC: { type: String, required:true },
    devices: {type: String},
    indoor: {type: Boolean},
    update: {type: Boolean, default: false },
    device_last_connect: {type: Number, default: 0},   // Ultima conexion
    plants: [{
        info: {
            name: {type: String},
            type: {type: String},
            description: {type: String,default: ""},
            index: {type: Number},
            date: {type: Number},   //Creation Date
            status: {type: Number, default: 0},   // Ultima conexion
            analog:
            {
                photocell1:{type: Number, default: 0 },
                photocell2:{type: Number, default: 0 },
                humCap:{type: Number, default: 0 },
                humEC:{type: Number, default: 0 },
                date:{ type: Date, default: 0 } 
            }
        },
        sowing:{
            light:{
                last_light:{ type: Number }, 
                time_start: {type: String},
                time_stop: {type: String},
                led_start: {type: Number},
                led_end: {type: Number},
                color_red: {type: Number},
                color_green: {type: Number},
                color_blue: {type: Number},
                status: {type: Boolean, default: false }
            },
            water:{
                last_water:{ type: Number }, 
                frequency:{ type:Number},
                supply: {type: Number},
                limit: {type: Number},
                pinout: {type: Number},
                status: {type: Boolean, default: false },
                open: {type: Number, default: 180 },
                close: {type: Number, default: 0 }
            },
            temperature:{
                min:{type: Number}
            }
        },
        pinout:{
            photocell1: {type: Number},
            photocell2: {type: Number},
            humCap: {type: Number},
            humEC: {type: Number}
        },
        others:{ type: Array}
    }],
    date:{ type: Date, default: Date.now } //User Creation date
})

UserSchema.methods.encryptPassword = async (password) => {
    const salt = await bcrypt.genSalt(10);
    const hash = bcrypt.hash(password, salt);
    return hash;
};

UserSchema.methods.matchPassword = async function(password) {
    return await bcrypt.compare(password, this.password);
};

module.exports = mongoose.model('User', UserSchema)