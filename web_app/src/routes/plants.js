const express = require('express');
const router = express.Router();
const Aromatics = require('../flowers/Aromaticas');
const vegetable = require('../flowers/Hortalizas');
const { isAuthenticated } = require("../helpers/auth")
const User = require('../models/User');

router.get('/my-plants',isAuthenticated, async(req,res)=>{
    
    const plants = {};
    const user = await User.findOne({email: req.user.email});
    
    user.plants.forEach( (element, index )=> {
        const plant =  JSON.parse(element);
        delete plant.sowing;
        
        if(plant.info.type === "vegetable"){
            info_copy = (vegetable[ (plant.info.index) ]).info;
        }else{
            info_copy = (Aromatics[ (plant.info.index) ]).info;
        }
    
        plant.info = info_copy;
        plants[index] = plant;
    }); 

    res.render('plants/plants', {plants});
});

router.get('/new-plant',isAuthenticated, (req,res)=>{
    res.render('plants/newPlant', {Aromatics, vegetable});
});

router.get('/new-plant/:Type/:index',isAuthenticated, (req,res)=>{
    const index = parseInt(req.params.index , 10);
    const Type = req.params.Type;
    let plant;

    if (Type == "aromatic"){
        plant = Aromatics[index];
    }else if(Type == "vegetable") {
        plant = vegetable[index];
    }else{
        res.render('404');
    }

    res.render('plants/newPlant-edit', {plant,index,Type});
});

router.post('/new-plant/:Type/:index/save',isAuthenticated, async (req,res)=>{
    const Index = parseInt(req.params.index , 10);
    const Type = req.params.Type;
    const { waterF, waterU, lightH, lightU} = req.body;
    let plant;
    
    if (Type == "aromatic"){
        plant = JSON.stringify(Aromatics[Index]);

    }else if(Type == "vegetable") {
        plant = JSON.stringify(vegetable[Index]);
    }else{
        res.render('404');
    }

    plant = JSON.parse(plant);
    plant.info = { type: Type,
                   index: Index } 
    plant.sowing.water.frequency = waterF;
    plant.sowing.water.limit = waterU;
    plant.sowing.light.min= lightH;
    plant.date = Date.now();
    
    await User.findByIdAndUpdate(req.user.id, {'$push': {plants: JSON.stringify(plant)} },{ new: true });
        
    req.flash("success_msg","Data update succefully");
    res.redirect('/my-plants');
});

router.get('/edit-plant/:date/:index/',isAuthenticated, async(req,res) =>{
    const date = req.params.date;
    const index = req.params.index;
    const user = await User.findOne({email: req.user.email});
    const plant = JSON.parse(user.plants[index]);
    
    res.send(plant);
 });

module.exports= router;



