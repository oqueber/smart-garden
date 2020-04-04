
let Flowers = [];

Flowers.push({
    Info:{
        Name: "Albahaca",
        Description: "The Albacaha is so delicius", 
        Recomendacion: "La Albahaca es una planta que no soporta para nada las heladas, por lo que se recomienda empezar la siembra en semilleros y luego transplantarlas",
        Beginner: false,
        Germinacion: 15, //Dias
        Siembra_Directa: false,
        Transplante: 45, //Dias
        Recoleccion: 50, // Dias
        Marco_Plantacion: "30x20", //cm
        Luz: 8, //Horas diarias de luz directa
        Web: "https://www.lahuertinadetoni.es/sembrar-germinar-la-albahaca-casa/"
    },
    Siembra: {
        Inicio: 4,       // (Mayo) Numero del mes empezando desde cero
        Fin: 5,          // (Junio) Numero del mes empezando desde cero
        Riego:{
            Frecuencia: 3, //Dias
            Umbral: 1
        },
        Temperatura:{
            Min: 15 //Grados
        }
    },
    Transplante:{
        Inicio: 5,          // (Junio) Numero del mes empezando desde cero
        Fin: 7,             // (Agosto) Numero del mes empezando desde cero
        Riego:{
            Frecuencia: 4, //Dias
            Umbral: 2 // Esperar que se seque la tierra hata el humbral: Tipo 0 seco, Tipo 1 un poco humedo, tipo 2 humedo, tipo 3 enchargado.
        }
    },
    Floracion:{
        Inicio: 7,  // (Agoto) Numero del mes empezando desde cero
        Fin:    9   // (Octubre) Numero del mes empezando desde cero
    }
});

Flowers.push({
    Info: {
        Name: "Perejil",
        Description: "The Perejil is so delicius", 
        Recomendacion: "Poner la semilla en remojo 24 horas.",
        Beginner: true,
        Germinacion: 15, //Dias
        Siembra_Directa: true,
        Recoleccion: 90, // Dias
        Luz: 3, //Horas minima de luz  diaria directa
        Marco_Plantacion: "30x20", //cm
        Web: "https://www.lahuertinadetoni.es/plantar-perejil-huerto-maceta/",
    },
    Siembra: {
        Inicio: 0,       // (Enero) Numero del mes empezando desde cero
        Fin: 11,          // (Diciembre) Numero del mes empezando desde cero
        Riego:{
            Frecuencia: 2, //Dias
            Umbral: 3 // Esperar que se seque la tierra hata el humbral: Tipo 0 seco, Tipo 1 un poco humedo, tipo 2 humedo, tipo 3 enchargado.
        },
        Temperatura:{
            Min: 15 //Grados
        }
    },
    Floracion:{
        Inicio: 0,  // (Enero) Numero del mes empezando desde cero
        Fin:    11   // (Diciembre) Numero del mes empezando desde cero
    }
});


module.exports= Flowers;