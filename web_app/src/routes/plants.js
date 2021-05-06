const express = require('express');
const router = express.Router();
const ManualPlant = require('../flowers/Manual');
const Aromatics = require('../flowers/Aromaticas');
const vegetable = require('../flowers/Hortalizas');
const { isAuthenticated } = require("../helpers/auth")
const User = require('../models/User');

router.get('/my-plants',isAuthenticated, async(req,res)=>{
    
    const user = await User.findOne({email: req.user.email});

    user.plants.forEach(plant => {
        //console.log(plant);
    
        if (plant.info.type == "aromatic"){
            plant.info.image = Aromatics[plant.info.index].info.image;
        }else if(plant.info.type  == "vegetable") {
            plant.info.image = vegetable[plant.info.index].info.image;
        }else{
            plant.info.image = "Manual_Plant.jpg";
        }
    });
    res.render('plants/home', {plants: user.plants});
});

router.get('/new-plant',isAuthenticated, (req,res)=>{
    res.render('plants/select', {Aromatics, vegetable});
});

router.get('/new-plant/:Type/:index',isAuthenticated, (req,res)=>{
    const index = parseInt(req.params.index , 10);
    const Type = req.params.Type;
    let plant;

    if (Type == "aromatic")
    {
       
        plant =  JSON.parse(JSON.stringify(Aromatics[index]));
    }
    else if(Type == "vegetable")
    {
        plant =  JSON.parse(JSON.stringify(vegetable[index])) ;
    }
    else{
        res.render('404');
    }

    res.render('plants/edit', {plant ,index,Type});
});
router.get('/delete-plant/:date', isAuthenticated, async(req,res) =>{
    const date = req.params.date;

    await User.findByIdAndUpdate(req.user.id,{
        $pull: {plants: { 'info.date': date}}
    });

    req.flash("success_msg","Data update succefully");
    res.redirect('/my-plants');
});
router.get('/edit-plant/:index/',isAuthenticated, async(req,res) =>{
    
    const index = parseInt(req.params.index,10);
    const user = await User.findOne({email: req.user.email});
    let plant = user.plants[index];
    plant["pinout/adc1"]=plant["pinout/photocell1"];
    plant["pinout/adc2"]=plant["pinout/photocell2"];
    plant["pinout/adc3"]=plant["pinout/humCap"];
    plant["pinout/adc4"]=plant["pinout/humEC"];

    
    res.render('plants/edit',{plant ,index, date: plant.info.date});
 });

router.post('/my-plants/new-plant', async(req,res) =>{
    let {title,description} = req.body;
    let errors = [];
    console.log(req.body);
    if(title == ''){errors.push({text: "Rellene el campo de titulo de planta"})}
    if(description == ''){errors.push({text: "Rellene el campo de descripcion de planta"})}


    if(errors.length >= 1){
        res.render('plants/select',{errors, Aromatics, vegetable});
    }else{
        const index = 0;
        const Type = "manual";
        let plant;
    
        plant = JSON.parse(JSON.stringify(ManualPlant[0]));
        plant.info.description = description;
        plant.info.name = title;
    
        res.render('plants/edit', {plant ,index,Type});
    }
});

router.post('/new-plant/:type/:index/save',isAuthenticated, async (req,res)=>{
    const errors =[];
    const index = parseInt(req.params.index , 10);
    const Type = req.params.type;
    const { waterF, waterU,waterPin, 
            color_red, color_blue,color_green, led_start,led_end, time_start, time_stop,
            adc1,adc2,adc3,adc4,
            name,waterOpen,waterClose,description} = req.body;
    //console.log(req.body);
    if( !Number.isInteger( parseInt(adc1,10) ) ){errors.push({text: "ADC1 necesita ser un numero"})}
    if( !Number.isInteger( parseInt(adc2,10) ) ){errors.push({text: "ADC2 necesita ser un numero"})}
    if( !Number.isInteger( parseInt(adc3,10) ) ){errors.push({text: "ADC3 necesita ser un numero"})}
    if( !Number.isInteger( parseInt(adc4,10) ) ){errors.push({text: "ADC4 necesita ser un numero"})}
    
    if( !Number.isInteger( parseInt(led_start,10) ) ){errors.push({text: "Definir inicio de led"})}
    if( !Number.isInteger( parseInt(led_end,10) ) ){errors.push({text: "Definir fin de led"})}
    if( !Number.isInteger( parseInt(waterOpen,10) ) ){errors.push({text: "Definir angulo de inicio de riego"})}
    if( !Number.isInteger( parseInt(waterClose,10) ) ){errors.push({text: "Definir angulo de cierre de riego"})}
    if( !Number.isInteger( parseInt(waterPin,10) ) ){errors.push({text: "Definir pin de riego"})}

    let plant = {
        update: true,
        info:{
            name: name,
            type: Type,
            index: index,
            image: "",
            description: description,
            date: Date.now()
        },
        sowing:{
            light:{
                last_light: Math.round(new Date() / 1000),
                time_start: time_start,
                time_stop: time_stop,
                led_start: parseInt(led_start,10),
                led_end: parseInt(led_end,10),
                color_red:  parseInt(color_red,10),
                color_green:parseInt(color_green,10),
                color_blue: parseInt(color_blue,10)
            },
            water:{
                last_water: Math.round(new Date() / 1000),
                frequency: parseInt(waterF,10),
                pinout: parseInt(waterPin,10),
                limit: parseInt(waterU,10),
                open: parseInt(waterOpen,10),
                close: parseInt(waterClose,10)
                
            },
            temperature:{
                min: parseInt('12',10)
            }
        },
        pinout:{
            photocell1: parseInt(adc1,10),
            photocell2: parseInt(adc2,10),
            humCap: parseInt(adc3,10),
            humEC: parseInt(adc4,10)
        }
    };
    

    if(errors.length >= 1){
        res.render('plants/edit', {plant ,index,Type, errors});
    }else{
        //console.log(plant.info);
        await User.findByIdAndUpdate(req.user.id, {'$push': {plants: plant } },{ new: true });
        req.flash("success_msg","Data update succefully");
        res.redirect('/my-plants');
    }
});


router.post('/edit-plant/:index/save',isAuthenticated, async(req,res)=>{
    const { waterF, waterU,time_start,time_stop, led_start, led_end,color_red,waterPin, color_blue,color_green ,adc1,adc2,adc3,adc4,waterOpen,waterClose} = req.body;
    const index = parseInt(req.params.index);
    await User.findOne( {_id:req.user.id }).then(doc => {
        
        doc.update = true;
        let plant = doc.plants[index];
        plant.sowing.light.color_red = parseInt(color_red,10);
        plant.sowing.light.color_green = parseInt(color_green,10);
        plant.sowing.light.color_blue = parseInt(color_blue,10);
        plant.sowing.light.led_start = parseInt(led_start,10);
        plant.sowing.light.led_end = parseInt(led_end,10);
        plant.sowing.light.time_start = time_start;
        plant.sowing.light.time_stop = time_stop;

        plant.sowing.water.frequency = parseInt(waterF,10);
        plant.sowing.water.limit = parseInt(waterU,10);
        plant.sowing.water.pinout = parseInt(waterPin,10);
        plant.sowing.water.open = parseInt(waterOpen,10);
        plant.sowing.water.close = parseInt(waterClose,10);

        plant.sowing.temperature.min = parseInt('14');
        plant.pinout.photocell1 = parseInt(adc1,10);
        plant.pinout.photocell2 = parseInt(adc2,10);
        plant.pinout.humCap = parseInt(adc3,10);
        plant.pinout.humEC = parseInt(adc4,10);
        //console.log(req.body); 
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



