
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
*              Water Umbral
* --------------------------------------------------*/
var lightH = document.getElementById('lightH');
noUiSlider.create(lightH, {
    start: [1],
    connect: 'lower',
    step: 1,
    range: {
        'min': [0],
        'max': [24]
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
var fillData = function(event) {
    //event.preventDefault();
    let form = document.getElementById('form');
    let data = {
        waterU: waterU.noUiSlider.get(),
        waterF: waterF.noUiSlider.get(),
        lightH: lightH.noUiSlider.get(),
        name: document.getElementById("PlantName").textContent,
        color_red: colors[0],
        color_green: colors[1],
        color_blue: colors[2]
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
};   

$( document ).ready(function() {
    // attach event listener
    form.addEventListener("submit", fillData, true)

});