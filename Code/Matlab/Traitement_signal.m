close all
clear all
clc



%[Freq800,Fe] = audioread('800.wav');
%Freq800 = Freq800(1:1000,1);

%[LA,FeLA] = audioread('_LA.wav');
%[LA,FeLA] = audioread('Enregistrement.m4a');
[LA,FeLA] = audioread('DO#.wav');

seuil = 0.04;
long_trame = 1024;
trame = zeros(1,long_trame);
n_trames = ceil(length(LA)/long_trame);
note_det = 0;
log_intensite = zeros(1,n_trames);

decalage = 256;
autocorr_trame = zeros(1,2*decalage+1);

deviation_peak_max = 6;
trame_periodique = zeros(1,long_trame);
% Analyse du son par trames
for i = 1:n_trames
    
    % Formation de la trame
    trame  = LA(long_trame*(i-1)+1:long_trame*i);
    
    % D�tection d'intensit�
    intensite = mean(abs(trame));
    log_intensite(i)=intensite;
    

    if(intensite>seuil)
        note_det=1;
        
        % Autocorr�lation
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
       
        
        % D�tection de peaks
        % TODO : S'assurer que la valeur absolue des peaks d�passe un
        % certain seuil � d�terminer
        n_peaks=0;
        for k = 4:(length(autocorr_trame)-3)
            if(autocorr_trame(k)>autocorr_trame(k-1) && autocorr_trame(k)>autocorr_trame(k-2) && autocorr_trame(k)>autocorr_trame(k-3))
                if(autocorr_trame(k)>autocorr_trame(k+1) && autocorr_trame(k)>autocorr_trame(k+2) && autocorr_trame(k)>autocorr_trame(k+3))
                    peaks(n_peaks+1)=k;
                    n_peaks=n_peaks+1;
                end
            end
        end
        
        % D�tection de la p�riodicit� des peaks
        periodique = 1;
        for k=2:(length(peaks)-1)
            ecart = abs((peaks(k)-peaks(k-1))-(peaks(k+1)-peaks(k)));
            if(ecart>deviation_peak_max)
                periodique = 0;
            end
        end
        trame_periodique(i) = periodique;
        
        % D�tection de la note
        
        
        figure(1)
        plot(autocorr_trame);
        hold on
        plot(peaks,autocorr_trame(peaks),'ro')
        
        
    end
end

figure(2)
plot(trame_periodique)
