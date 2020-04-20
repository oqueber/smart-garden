const express = require('express');
const socketio = require('socket.io');
const http = require('http');

const app= express();
const server = http.Server(app);
const io = new socketio(server);


module.exports  = {app, io, express, server}
