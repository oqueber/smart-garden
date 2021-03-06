const express = require('express');
const socketio = require('socket.io');
const http = require('http');

const app= express();
const server = http.Server(app);
const io = new socketio(server);

// Import events module
var events = require('events');
// Create an eventEmitter object
var eventEmitter = new events.EventEmitter();

let devicesConnected = new Map();
let userConnected = new Map();

module.exports  = {app, io, express, server, eventEmitter, devicesConnected,userConnected}
