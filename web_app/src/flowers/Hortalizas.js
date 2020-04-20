
let Hortalizas = [];


Hortalizas.push({
    Name: "Zanahoria",
    Description: "The Zanahoria is so delicius", 
    Beginner: true,
    Germinacion: 20, //Dias
    Siembra_Directa: true,
    Recoleccion: 90, // Dias
    Luz: 8, //Horas minima de luz  diaria directa

    Siembra: {
        Inicio: 0,       // (Enero) Numero del mes empezando desde cero
        Fin: 11,          // (Diciembre) Numero del mes empezando desde cero
        Riego:{
            Tierra_Seca: false, //No permitir que se seque el sustrato
            Frecuencia: [3,4,5] //Dias
        },
        Temperatura:{
            Min: 15 //Grados
        }
    },
    Floracion:{
        Inicio: 0,  // (Agoto) Numero del mes empezando desde cero
        Fin:    11   // (Octubre) Numero del mes empezando desde cero
    },
    info: {
        recomendacion: "Aclarar las plantas a 10cm de distancia",
        Marco_Plantacion: "30x15", //cm
        Web: "https://www.lahuertinadetoni.es/sembrar-germinar-la-albahaca-casa/",
        Video: "https://www.youtube.com/watch?v=a1MOu_vHOsQ",
        Comentario: "La Albahaca es una planta que no soporta para nada las heladas, por lo que se recomienda empezar la siembra en semilleros y luego transplantarlas"
    },
});

module.exports= Hortalizas;