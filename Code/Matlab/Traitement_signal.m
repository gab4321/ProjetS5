close all
clear all
clc



%[Freq800,Fe] = audioread('800.wav');
%Freq800 = Freq800(1:1000,1);

[LA,FeLA] = audioread('_LA.wav');
%[LA,FeLA] = audioread('Enregistrement.m4a');

seuil = 0.04;
long_trame = 1024;
trame = zeros(1,long_trame);
n_trames = ceil(length(LA)/long_trame);
note_det = 0;
log_intensite = zeros(1,n_trames);

decalage = 256;
autocorr_trame = zeros(1,2*decalage+1);
autocorr_entier = zeros
% Analyse du son par trames
for i = 1:n_trames
    
    % Formation de la trame
    trame  = LA(long_trame*(i-1)+1:long_trame*i);
    
    % Détection d'intensité
    intensite = mean(abs(trame));
    log_intensite(i)=intensite;
    

    if(intensite>seuil)
        note_det=1;
        
        % Autocorrélation
        % A faire sans la fonction xcorr
        % Il ne faut garder que quelques peaks possibles
        somme = 0;
        trame_pad = [zeros(1,decalage), trame, zeros(1,decalage)];
        for v = -decalage:decalage
            for n = 1:(long_trame - decalage)
                somme = somme + trame_pad(n+decalage)*trame_pad(n+v+decalage);
            end
            autocorr_trame(v+decalage+1) = somme;
        end
        plot(autocorr_trame)
        
        % Détection de peaks
        % Il faut tenir compte de la valeur absolue des peaks (>1)
        % Il faut vérifier la périodicité
        
        % Détection du premier peak
        
        

        
    end
end
