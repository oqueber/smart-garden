const express = require('express');
const router = express.Router();
const User = require('../models/User')


router.get('/', (req,res)=>{
    res.render('index');
});
router.get('/about', (req,res)=>{
    res.render('about');
});

router.get('/act/:date', (req,res)=>{
    const date = parseInt(req.params.date,10);
    const MAC = req.user.MAC;

    res.render('plants/act',{date,MAC});
});

/*  Data:{
*        HW:{
*            // BME280 CCS811 Si7021 TCS34725, un 1 en su lugar representa que se buscara esta metrica
*            Measurements: "1111", //Queremos todos los datos de estos sensores
*            Plants:{
*                Plant1:{ 
*                    HumCap: A0,
*                    HumEC: A1,
*                    Riego: 19 
*                },
*                Plant2:{
*                    HumCap: A2,
*                    HumEC: A3,
*                    Riego: 20 
*                }
*            }
*        }
*    }
*/
router.get("/Users/GetData/:Mac",async (req,res)=>{
    var plants = {};
    var _mac = req.params.Mac
    var _user = await User.findOne({"MAC":_mac});

    if(_user != null){
        if(_user.plants.length >= 1){
            
            console.log("Exits plants");
            
            _user.plants.forEach( (element, index) => {
                plants[element.info.date] = element.pinout;
            }); 

            console.log( `Send `,{Measurements: _user.devices,plants:plants});
            res.json({Measurements: _user.devices,plants:plants});
        }else{
            res.json({Measurements: "1111"});
        }
    }else{
        res.sendStatus(204);
    }
});


module.exports= router;



