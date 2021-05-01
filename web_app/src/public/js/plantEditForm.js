
/* --------------------------------------------------
*              Water Frequency
* --------------------------------------------------*/
var waterF = document.getElementById('waterF');

noUiSlider.create(waterF, {
    start: [1],
    connect: 'lower',
    step: 1,
    range: {
        'min': [0],
        'max': [28]
    }
});
/* --------------------------------------------------
*              Water Umbral
* --------------------------------------------------*/
var waterU = document.getElementById('waterU');
noUiSlider.create(waterU, {
    start: [1],
    connect: 'lower',
    step: 1,
    range: {
        'min': [0],
        'max': [3]
    }
});
/* --------------------------------------------------
*              Color
* --------------------------------------------------*/
var resultElement = document.getElementById('result');
var sliders = document.getElementsByClassName('sliders');
var colors = [0, 0, 0];
[].slice.call(sliders).forEach(function (slider, index) {

    noUiSlider.create(slider, {
        start: 127,
        connect: [true, false],
        orientation: "vertical",
        range: {
            'min': 0,
            'max': 255
        },
        format: {   
            from: function(value) {
            return parseInt(value);
        },
            to: function(value) {
            return parseInt(value);
        }}
    });
    // Bind the color changing function to the update event.
    slider.noUiSlider.on('update', function () {

        colors[index] = slider.noUiSlider.get();

        var color = 'rgb(' + colors.join(',') + ')';

        resultElement.style.background = color;
        resultElement.style.color = color;
    });
});
/* --------------------------------------------------
*              Send info
* --------------------------------------------------*/
let form = document.getElementById('myForm');
var fillData = function(event) {
    //event.preventDefault();
    let data = {
        waterU: waterU.noUiSlider.get(),
        waterF: waterF.noUiSlider.get(),
        name: document.getElementById("PlantName").textContent,
        color_red: colors[0],
        color_green: colors[1],
        color_blue: colors[2],
        description: localPlant.info.description
    };
    for (const key in data) {
        if (data.hasOwnProperty(key)) {
            const hiddenField = document.createElement('input');
            hiddenField.type = 'hidden';
            hiddenField.name = key;
            hiddenField.value = data[key];

            form.appendChild(hiddenField);
        }
    }
    console.log("Añadiedo datos:");
    console.log(data);
};   

$( document ).ready(function() {
    // attach event listener
    form.addEventListener("submit", fillData, true);

});