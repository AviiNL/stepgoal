// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig);

Pebble.addEventListener('ready', function () {
    Pebble.sendAppMessage(JSON.parse(localStorage.getItem('clay-settings')), function () {
        console.log('Config data sent successfully!');
    }, function (e) {
        console.log('Error sending config data!');
    });
});