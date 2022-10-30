# smart-lights
 
 The goal of this project is to create an intelligent lighting system that can be controlled by phone and virtual assistants

Attempting to connect to the Sinric Pro servers, if the connection is not successful it is retried, if successful the microcontroller asks for the last status of the relays, turns on the lights according to the last values in the status matrix, then saves the status of the switches in the matrix according to time, if no change has occurred, the matrix is updated with the new time, if a change has occurred, the lights turn on, changing the status of the relays accordingly, the status matrix is updated with the new values and the new time, the last status is sent to the server, following the process restart .