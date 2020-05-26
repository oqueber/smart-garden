const express = require('express');
const router = express.Router();
const { isAuthenticated } = require("../helpers/auth")

router.get('/chart', isAuthenticated, (req,res)=>{
    const mac = req.user.MAC;
    console.log(`sending ${mac}`);
    res.render('charts/main_chart', {mac} );
});


router.get('/notes', isAuthenticated,async (req,res)=>{
    const notes = await Note.find().sort({date: 'desc'});
   console.log(notes);
   res.render('notes/all-notes',{ notes });
});

module.exports= router;