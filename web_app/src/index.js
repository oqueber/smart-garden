const { app, express, server} = require('./utils/socketio.js');
const connectdb = require('./database');
const Mqtt = require('./mqtt/server');
const Socket = require('./socketio/server');
const flash = require('connect-flash');
const passport = require('passport');
const path = require('path');
const exphbs = require('express-handlebars');
const methodOverride= require('method-override');
const session = require('express-session');
const debug = require('debug')("SG:Server-Main");
const chalk = require('chalk');
const ip = "34.132.72.212";

//Initializations
process.title = 'myApp';
connectdb.then( db => {
    debug( chalk.blue( `[MongoDB]: Connected` ));
})
require('./config/passport');

//-----------------------------------------
//------------ Settings  ------------------
//-----------------------------------------
const port =process.env.PORT || 80;
app.set('views',path.join(__dirname,'views'));
require('./config/passport')

// Diseño de las plantillas HTML
app.engine('.hbs', exphbs({
    defaultLayout: 'main', // Pagina principal
    layoutsDir: path.join(app.get('views'),'layouts'),  //Plantilla principal
    partialsDir: path.join(app.get('views'),'partials'),//Pequeñar partes de HTML que se pueden reutilizar
    extname: '.hbs', //Que extrensiones tendran nuestros archivos

    helpers:{
        JSON2string: function (object) {
            return JSON.stringify(object);
        },
        toDate: function(date){
            if ( date == 0 || date == null){
                return "Sin datos";
            }else{
                let date_aux = new Date(date).toJSON();
                if( date_aux.slice(0,4) == "1970" )
                {
                    return "Sin datos";  
                }

                return (date_aux.slice(11,16)+" "+//Hora  
                        date_aux.slice(8,10)+"-"+ // Dia
                        date_aux.slice(5,7)+"-"+  //Mes 
                        date_aux.slice(2,4)  //año
                        );  
            }
        },
        toPercentage: function(data)
        {
            if ( data == 0){
                return "0";
            }else{
                return Math.round( (data/4096)*100 ) ;
            }
        },
        isVegetable: function (value) {
            return value == "vegetable";
        }
    }
}))
app.set('view engine', '.hbs'); // Configuramos el mortor de las plantillas
app.set('view options', { layout: 'webside' });
//-----------------------------------------
//------------ Middlewares  ---------------
//-----------------------------------------
app.use(express.urlencoded({extended: false}))
app.use(methodOverride('_method'));
app.use(session({
    secret: 'mysecreapp',
    resave: true,
    saveUninitialized: true
}));
app.use(passport.initialize());
app.use(passport.session());
app.use(flash());

// Global variables
app.use((req, res, next) => {
    res.locals.success_msg = req.flash('success_msg');
    res.locals.error_msg = req.flash('error_msg');
    res.locals.error = req.flash('error');
    res.locals.user = req.user || null;
    next();
});

//-----------------------------------------
//------------ Routes  --------------------
//-----------------------------------------
app.use(require('./routes/index'));
app.use(require('./routes/chart'));
app.use(require('./routes/users'));
app.use(require('./routes/plants'));


// Static Files
app.use(express.static(path.join(__dirname, 'public')));

//-----------------------------------------
//------------ Server is listenning -------
//-----------------------------------------
//server.listen(port, ip, () =>{
server.listen(port, () =>{
    debug( chalk.blue(`[Plataforma web] Connected to ${ip}:${port}`));
});
