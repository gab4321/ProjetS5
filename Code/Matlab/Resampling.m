clear all
clc
close all

[note,fs]=audioread('Accords/Ginv.m4a');

note_8 = decimate(note,6);
newfs=fs/6;
audiowrite('Accords/Ginv.wav',note_8,newfs);
