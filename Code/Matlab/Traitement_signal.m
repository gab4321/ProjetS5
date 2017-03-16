close all
clear all
clc



%[Freq800,Fe] = audioread('800.wav');
%Freq800 = Freq800(1:1000,1);

% [note_audio,Fe] = audioread('Enregistrement.m4a');
%
%
% [note_audio,Fe] = audioread('DO5.wav'); 
% [note_audio,Fe] = audioread('DO#.wav');
% [note_audio,Fe] = audioread('Re.wav');
% [note_audio,Fe] = audioread('Re#.wav');
% [note_audio,Fe] = audioread('Mi.wav'); 
% [note_audio,Fe] = audioread('Fa.wav');
% [note_audio,Fe] = audioread('_Fa#.wav');
% [note_audio,Fe] = audioread('_sol.wav'); 
% [note_audio,Fe] = audioread('_sol#.wav');
% [note_audio,Fe] = audioread('_LA.wav');
% [note_audio,Fe] = audioread('LA#.wav'); 
% [note_audio,Fe] = audioread('Si.wav');
%
 [note_audio,Fe] = audioread('Gamme majeur Do.wav'); % problème à 267



seuil = 0.02;
long_trame = 2048;
trame = zeros(1,long_trame);
% n_trames = ceil(length(note_audio)/long_trame);
 n_trames = 266;
note_det = 0;
log_intensite = zeros(1,n_trames);

decalage = 256;
autocorr_trame = zeros(1,2*decalage+1);


deviation_ecart_peak_max = 6;
log_periodique = zeros(1,n_trames);
n_trames_son = 0;
n_trames_to_skip = 5;
n_trames_to_keep = 2; % À NE PAS CHANGER. Seulement pour la détec. périod.

freq_trames=zeros(1,n_trames);
% Analyse du son par trames
for n_trame = 1:n_trames
    
    % Formation de la trame
    trame  = note_audio(long_trame*(n_trame-1)+1:long_trame*n_trame);
    
    % Détection d'intensité
    intensite = mean(abs(trame));
    log_intensite(n_trame)=intensite;
    
    if(intensite>seuil) % Si c'est un son
        
        n_trames_son = n_trames_son + 1;
        if(n_trames_son>n_trames_to_skip)
            n_trame_analyse = n_trames_son-n_trames_to_skip;
            
            % Filtrage de la trame
            % TODO
            
            if(n_trame_analyse<=n_trames_to_keep)
                % Autocorrélation
                somme = 0;
                trame_pad = [zeros(1,decalage), trame, zeros(1,decalage)];
                for v = -decalage:decalage
                    for n = 1:(long_trame - decalage)
                        somme = somme + trame_pad(n+decalage)*trame_pad(n+v+decalage);
                    end
                    autocorr_trame(v+decalage+1) = somme;
                end
                
                
                % Détection de peaks
                n_peaks=0;
                peaks = 0;
                for k = 4:(length(autocorr_trame)-3)
                    if(autocorr_trame(k)>autocorr_trame(k-1) && autocorr_trame(k)>autocorr_trame(k-2) && autocorr_trame(k)>autocorr_trame(k-3))
                        if(autocorr_trame(k)>autocorr_trame(k+1) && autocorr_trame(k)>autocorr_trame(k+2) && autocorr_trame(k)>autocorr_trame(k+3))
                            peaks(n_peaks+1)=k;
                            n_peaks=n_peaks+1;
                        end
                    end
                end
            end
            
            % Si c'est les premiers peaks, initialiser le vecteur de log
            % pour des raisons d'efficacité. On ne garde que deux
            if((n_trame_analyse)==1)
                log_peaks=zeros(n_trames_to_keep,n_peaks);
            end
            
            
            % Détection de la périodicité
            if(n_trame_analyse==1) % premier échantillon
                log_peaks(1,:)=peaks;
                periodique=0;
            elseif(n_trame_analyse==2) %second
                if(length(peaks)~=length(log_peaks(1,:)))
                    periodique=0;
                else
                    
                    log_peaks(2,:)=peaks;
                    periodique=1;
                    for k=1:length(log_peaks)
                        if(abs(log_peaks(1,k)-log_peaks(2,k))>deviation_ecart_peak_max)
                            periodique=0;
                        end
                    end
                    log_periodique(n_trame-1:n_trame)=periodique*ones(1,2);
                end
            else
                log_periodique(n_trame)=periodique;
            end
            
            
            % Détection de la note
            if(periodique)
                FFT_trame = fft(hanning(long_trame)'.*trame);
                mag_FFT_trame =  abs(FFT_trame);
                [amax,imax] = max(mag_FFT_trame);
                freq_trames(n_trame) = (imax-1)/(long_trame)*Fe;
                if(n_trame_analyse==2)
                    freq_trames(n_trame-1) = freq_trames(n_trame);
                end
            end
            
            figure(1)
            plot(autocorr_trame);
            hold on
            plot(peaks,autocorr_trame(peaks),'ro')
            title('Autocorrélation et détection de peaks')
            
            peaks;
            
        end
    else % si l'intensité est sous le seuil
        n_trames_son=0;
    end
end

figure(2)
area(log_periodique)
title('Périodicité des trames')

figure(3)
%plot(trame_periodique.*freq_trames)
plot(freq_trames)
title('Fréquence des trames')

figure(4)
plot(log_intensite)
title('Intensité des trames')



%% Fonctions

