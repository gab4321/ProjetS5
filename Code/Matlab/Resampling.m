clear all
clc
close all

[note,fs]=audioread('Arpege do plus accord do maj do maj re maj re maj mi min mi min mi maj do inversé.m4a');

note_8 = decimate(note,6);
newfs=fs/6;
audiowrite('Accords/Arpege_mongol.wav',note_8,newfs);
