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
    plants: [{
        info: {
            type: {type: String},
            index: {type: Number},
            date: {type: Number}
        },
        sowing:{
            light:{
                hours: {type: Number},
                color: {type: String}
            },
            water:{
                frequency:{ type:Number},
                limit: {type: Number}
            },
            temperature:{
                min:{type: Number}
            }
        },
        pinout:{
            ADC1: {type: Number},
            ADC2: {type: Number},
            ADC3: {type: Number},
            ADC4: {type: Number}
        },
        others:{ type: Array}
    }],
    date:{ type: Date, default: Date.now }
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