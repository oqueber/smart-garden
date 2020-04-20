const express = require('express');
const router = express.Router();
const User = require('../models/User')


router.get('/', (req,res)=>{
    res.render('index');
});
router.get('/about', (req,res)=>{
    res.render('about');
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
router.get("/Users/HW/:Mac",async (req,res)=>{
    var _mac = req.params.Mac
    var _user = await User.findOne({"MAC":_mac});
    console.log( `Receved ${_mac}`);
    console.log( `Send `,_user.Data.HW);
    res.json(_user.Data.HW);
});


module.exports= router;



