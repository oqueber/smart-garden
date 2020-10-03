const express = require('express');
const router = express.Router();
const User = require('../models/User');
const { isAuthenticated } = require("../helpers/auth")

router.get('/chart', isAuthenticated, async (req,res)=>{
    const mac = req.user.MAC;
    var user = await User.findOne({"MAC":mac});
    var plants = [];
    user.plants.forEach((plant,index )=> {     
        plants.push( {Id: plant.info.date, Number: index} )
    });
    
    res.render('charts/main_chart', {mac,plants} );
});


module.exports= router;