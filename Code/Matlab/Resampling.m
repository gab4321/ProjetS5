clear all
clc
close all

[note,fs]=audioread('Gamme majeur Do.wav');

note_8 = decimate(note,6);
newfs=fs/6;
audiowrite('Gamme_majeur_Do_8.wav',note_8,newfs);
