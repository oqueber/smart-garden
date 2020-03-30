
const Flowers;

Flowers["Albahaca"] ={
    Directa: false,
    Germinacion: 15, //Dias
    Transplante: 45, //Dias
    Luz: 8, //Horas diarias de luz directa
    Siembra: {
        Inicio: 4,       // (Mayo) Numero del mes empezando desde cero
        Fin: 5,          // (Junio) Numero del mes empezando desde cero
        Riego:{
            Tierra_Seca: false, //No permitir que se seque el sustrato
            Frecuencia: [3,4,5] //Dias
        },
        Temperatura:{
            Min: 15 //Grados
        }
    },
    Transplante:{
        Inicio: 5,          // (Junio) Numero del mes empezando desde cero
        Fin: 7,             // (Agosto) Numero del mes empezando desde cero
        Riego:{
            Tierra_Seca: true,
            Frecuencia: [4,5] //Dias
        }
    },
    Floracion:{
        Inicio: 7,  // (Agoto) Numero del mes empezando desde cero
        Fin:    9   // (Octubre) Numero del mes empezando desde cero
    },
    info: {
        web: "https://www.lahuertinadetoni.es/sembrar-germinar-la-albahaca-casa/",
        Video: "https://www.youtube.com/watch?v=a1MOu_vHOsQ",
    },
    Comentario: "La Albahaca es una planta que no soporta para nada las heladas, por lo que se recomienda empezar la siembra en semilleros y luego transplantarlas"
}

Flowers["Perejil"] ={
    Beginner: true,
    Siembra_Directa: true,
    Germinacion: 90, //Dias
    Luz: 3, //Horas minima de luz  diaria directa
    Siembra: {
        Inicio: 0,       // (Enero) Numero del mes empezando desde cero
        Fin: 11,          // (Diciembre) Numero del mes empezando desde cero
        Riego:{
            Tierra_Seca: false, //No permitir que se seque el sustrato
            Humbral: 1 // Tipos de humbral: Tipo 0 seco, Tipo 1 un poco humedo, tipo 2 humedo, tipo 3 enchargado
        },
        Temperatura:{
            Min: 15 //Grados
        }
    },
    Transplante:{
        Inicio: 5,          // (Junio) Numero del mes empezando desde cero
        Fin: 7,             // (Agosto) Numero del mes empezando desde cero
        Riego:{
            Tierra_Seca: true,
            Frecuencia: [4,5] //Dias
        }
    },
    Floracion:{
        Inicio: 7,  // (Agoto) Numero del mes empezando desde cero
        Fin:    9   // (Octubre) Numero del mes empezando desde cero
    },
    info: {
        web: "https://www.lahuertinadetoni.es/plantar-perejil-huerto-maceta/",
    },
    Comentario: "La Albahaca es una planta que no soporta para nada las heladas, por lo que se recomienda empezar la siembra en semilleros y luego transplantarlas"
}

module.exports= Flowers;