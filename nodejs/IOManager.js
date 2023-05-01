/** ********************************************
 * nome file: IOManager.js
 * contenuto: 
 *    modulo di gestione della comunicazione con i muduli 
 *    di espansione IO 
 * autore: Glauco Bordoni
 * data: 2023-03-09
 * ********************************************** */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

"use strict" 

//load modules
var ipc=require('node-ipc').default;

//global vars
var cfg={}
var socketId = 'icp-kernel';


/** 
 * globals variables to store the callback function and the HTTP connection to send the data 
 */
var callback
var resHTTP
var busy=false;


/* globals variables to manage the polling */
let comandoPolling=[]
let pollingTime
var pollingTimer=null
var pollingAttivo=false;
let pollingRes;


/**
 * entry point
 */
function run(configurazione=null) {
    if (configurazione==null)
		cfg=configurazione
	else
		cfg.verbose=true;

	log("module IOManager.run")

    log("Starts the  IPC client of IOManger")
    
	ipc.config.id   = socketId;
	ipc.config.socketRoot = '/tmp/';
	ipc.config.appspace = '';
	ipc.config.rawBuffer=true;
	ipc.config.encoding='utf8';
	ipc.config.silent=(cfg.verbose!=true)

	ipc.config.retry= 1500;
	
	ipc.connectTo(
	  socketId,
	  function(){
		ipc.of[socketId].on(
			'connect',
			function (){
				console.log("IOManger Connected!!");
				ipc.log('## connected to world ##'.rainbow, ipc.config.delay);
			}
		);
		ipc.of[socketId].on(
		  'disconnect',
		  function(){
			console.log("IOManger Disconnected!!");
			ipc.log('disconnected from world'.notice);
		  }
		);
		ipc.of[socketId].on(
			'data',  //any event or message type your server listens for
			function (data){
				riceveRisposta(data)
			}
		);
	  }
	);    
}


/**
 * receive the command to send on the ipc-socket.
 * the command is trasformed into comma separated string
 * 
 * @param {*} comando the command object 
 * @param {*} clb the callback function
 * @param {*} res othe params (ie an 'express' request)
 */
async function inviaComando(comando,clb,res){

	log("IOManger: inviaComando");
	log(comando);

	while( busy)
		await sleep(300)

	busy=true;

	callback=clb
	resHTTP=res

	ipc.of[socketId].emit(
		comando
	);
}

/** 
 * Riceives the data from the IPC and calls the callback function set by inviaComando fucntion
 * using the HTTP connection on wich rend the receive String 
 * 
 * @param {*} data the Buffer with the data to be interpreted as JSON
 */
function riceveRisposta(data){
	log("IOManger: riceveRisposta ");

	let risposta=JSON.parse(data.toString())

	if (callback!=null)
		callback(risposta,resHTTP);
	busy=false;
	restartPolling();
}

/*********************************************************************************
 * 
 * Polling functionalities
 * 
 ********************************************************************************* */
						  
/**
 * it starts the polling, receive the command to poll adn the time to wait for the nex pool
 * @param {*} comando the command to be send  
 * @param {*} millisec the time to wait before to start a new poll
 */
function activatePolling(comando, millisec){
	log("IOManger: activatePolling(): enable polling of: "+comando);
	comandoPolling[0]=comando
	pollingTime=millisec
	if (pollingAttivo) // stava già eseguendo il polling
		return	//termina
	pollingAttivo=true
	pollingTimer=setInterval(polling,pollingTime)
}


/**
 * execute the polling if the flag pollingAttivo==true
 */
function polling(){
	if (pollingAttivo) //se non è in pausa
		inviaComando(comandoPolling[0],storePolingRes,null)
}


/**
 * stops the polling and set the flag pollingAttivo=false
 */
function deactivatePolling(){
	log("IOManger: deactivatePolling(): disable polling");
	
	clearInterval(pollingTimer)
	pollingTimer=null
	pollingAttivo=false
}


/**
 * pause the polling setting to false the flag pollingAttivo 
 */
function pausePolling(){
	log("IOManger: pausaPolling(): set polling in pause");
	if (pollingTimer!=null)
		pollingAttivo=false
}


/**
 * restart the polling previously paused setting  pollingAttivo=true 
 */
function restartPolling(){
	log("IOManger: restartPolling(): resume polling");
	if (pollingTimer!=null)
		pollingAttivo=true
}


/**
 * stores the answer received via socket
 * @param {*} risposta 
 */
function storePolingRes(risposta){
	pollingRes=risposta
}


/**
 * returns the last answer received via socket
 * @returns the answer
 */
function getPollingRes(){
	return pollingRes
}


/** 
 * Console log function 
 * s string passed into the function
*/
function log(s){
    if (cfg.verbose)
        console.log(s);
}


/**
 * stop the app for ms milliseconds
 * @param {*} ms millsieconds to wait
 * @returns 
 */
function sleep(ms) {
	return new Promise((resolve) => {
	  setTimeout(resolve, ms);
	});
}


//IPC management
exports.run= run;
exports.inviaComando= inviaComando;

//polling management
exports.activatePolling   = activatePolling
exports.deactivatePolling = deactivatePolling
exports.getPollingRes     = getPollingRes
