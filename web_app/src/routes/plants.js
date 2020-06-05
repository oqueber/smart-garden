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
        let info_copy;
        
        if(element.info.type == "vegetable"){
            info_copy = JSON.stringify(vegetable[ (element.info.index) ]);
        }else if(element.info.type == "aromatic"){
            info_copy = JSON.stringify(Aromatics[ (element.info.index) ]);
        }else{
            res.render('404');
        }
        let plant = JSON.parse(info_copy).info;
        plant['date'] = element.info.date;

        plants[index] = {info: plant};
    }); 
    res.render('plants/home', {plants});
});

router.get('/new-plant',isAuthenticated, (req,res)=>{
    res.render('plants/select', {Aromatics, vegetable});
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

    res.render('plants/edit', {plant ,index,Type});
});

router.get('/edit-plant/:index/',isAuthenticated, async(req,res) =>{
    
    const index = parseInt(req.params.index,10);
    const user = await User.findOne({email: req.user.email});
    //const user = await User.findOne({email: "luis@gmail.com"});
    let plant = user.plants[index];
    let info_copy;
    
    if(plant.info.type == "vegetable"){
        info_copy = (vegetable[ plant.info.index]).info;
    }else if(plant.info.type == "aromatic"){
        info_copy = (Aromatics[ plant.info.index]).info;
    }else{
        res.render('404');
    }
    
    res.render('plants/edit',{plant ,info:info_copy  ,index});
 });
 
router.post('/new-plant/:type/:index/save',isAuthenticated, async (req,res)=>{
    const errors =[];
    const index = parseInt(req.params.index , 10);
    const Type = req.params.type;
    const { waterF, waterU, lightH, color ,adc1,adc2,adc3,adc4,name} = req.body;
    if( !Number.isInteger( parseInt(adc1,10) ) ){errors.push({text: "ADC1 neeeds to be a number"})}
    if( !Number.isInteger( parseInt(adc2,10) ) ){errors.push({text: "ADC2 neeeds to be a number"})}
    if( !Number.isInteger( parseInt(adc3,10) ) ){errors.push({text: "ADC3 neeeds to be a number"})}
    if( !Number.isInteger( parseInt(adc4,10) ) ){errors.push({text: "ADC4 neeeds to be a number"})}
    let plant = {
        info:{
            type: Type,
            index: index,
            date: Date.now()
        },
        sowing:{
            light:{
                hours: parseInt(lightH),
                color: color
            },
            water:{
                frequency: parseInt(waterF),
                limit: parseInt(waterU)
            },
            temperature:{
                min: parseInt('12')
            }
        },
        pinout:{
            ADC1: parseInt(adc1),
            ADC2: parseInt(adc2),
            ADC3: parseInt(adc3),
            ADC4: parseInt(adc4)
        }
    };

    if(errors.length >= 1){
        plant.info.name = name;
        res.render('plants/edit', {plant ,index,Type, errors});
    }else{

        await User.findByIdAndUpdate(req.user.id, {'$push': {plants: plant } },{ new: true });
        req.flash("success_msg","Data update succefully");
        res.redirect('/my-plants');
    }
});


router.post('/edit-plant/:index/save',isAuthenticated, async(req,res)=>{
    const { waterF, waterU, lightH, color ,adc1,adc2,adc3,adc4} = req.body;
    const index = parseInt(req.params.index);
    await User.findOne( {_id:req.user.id }).then(doc => {

        let plant = doc.plants[index];
        plant.sowing.light.hours = parseInt(lightH,10);
        plant.sowing.light.color = color;
        plant.sowing.water.frequency = parseInt(waterF,10);
        plant.sowing.water.limit = parseInt(waterU,10);
        plant.sowing.temperature.min = parseInt('12');
        plant.pinout.ADC1 = parseInt(adc1,10);
        plant.pinout.ADC2 = parseInt(adc2,10);
        plant.pinout.ADC3 = parseInt(adc3,10);
        plant.pinout.ADC4 = parseInt(adc4,10);
        console.log(plant); 
        doc.save();
      
        //sent respnse to client
      }).catch(err => {
        console.log(err);
        console.log('Oh! Dark')
        res.send(err);
      });
    req.flash("success_msg","Data update succefully");
    res.redirect('/my-plants');
});



module.exports= router;



