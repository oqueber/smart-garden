const express = require('express');
const router = express.Router();
const User = require('../models/User')


router.get('/', (req,res)=>{
    res.render('index',{ title: 'Index', layout: 'webside' });
});
router.get('/about', (req,res)=>{
    res.render('about',{ title: 'about', layout: 'webside' });
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
    var fl_update = 0;

    await User.findOne( {"MAC":_mac} ).then( _user => {
        
        //Update the flag to false
        if ( _user.update == true) {
            console.log("Habia datos por actualizar");
            fl_update = 1;
            _user.update = false;
            _user.save();
        }

        if(_user.plants.length >= 1){
            
            //console.log("Exits plants");
            
            _user.plants.forEach( (element) => {
                plants[element.info.date] = {
                                            light: element.sowing.light,
                                            water: element.sowing.water,
                                            temperature: element.sowing.temperature,
                                            pinout: element.pinout
                                            };
            }); 


            //console.log(plants);
            //console.log( `Send `,{Measurements: _user.devices,plants:plants});
            res.json({Measurements: _user.devices, update:fl_update, plants:plants});
        }else{
            res.json({Measurements: "1111"});
        }
      
    //sent respnse to client
    }).catch(err => {
        console.log(err);
        console.log('Oh! Dark')
        res.send(err);
    });

    //res.sendStatus(204);
});


module.exports= router;



