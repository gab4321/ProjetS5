close all
clear all
clc



%[Freq800,Fe] = audioread('800.wav');
%Freq800 = Freq800(1:1000,1);

% [note_audio,Fe] = audioread('Enregistrement.m4a');
% 
% 
% [note_audio,Fe] = audioread('DO5.wav'); %Pas correct
% [note_audio,Fe] = audioread('DO#.wav');
% [note_audio,Fe] = audioread('Re.wav');
% [note_audio,Fe] = audioread('Re#.wav');
% [note_audio,Fe] = audioread('Mi.wav'); % Pas correct
% [note_audio,Fe] = audioread('Fa.wav');
% [note_audio,Fe] = audioread('_Fa#.wav');
 [note_audio,Fe] = audioread('_sol.wav'); % Pas correct
% [note_audio,Fe] = audioread('_sol#.wav');
% [note_audio,Fe] = audioread('_LA.wav');
% [note_audio,Fe] = audioread('LA#.wav'); % Fonctionne, mais peaks en bas
% [note_audio,Fe] = audioread('Si.wav');
% 
% [note_audio,Fe] = audioread('Gamme majeur Do.wav'); % Combination d'aberrations



seuil = 0.04;
long_trame = 2048;
trame = zeros(1,long_trame);
n_trames = ceil(length(note_audio)/long_trame);
note_det = 0;
log_intensite = zeros(1,n_trames);

decalage = 256;
autocorr_trame = zeros(1,2*decalage+1);


deviation_ecart_peak_max = 6;
trame_periodique = zeros(1,n_trames);
n_trames_son = 0;
n_trames_to_skip = 4;
n_trames_to_keep = 2; % À NE PAS CHANGER. Seulement pour la détec. périod.

freq_trames=zeros(1,n_trames);
% Analyse du son par trames
for i = 1:n_trames
    
    % Formation de la trame
    trame  = note_audio(long_trame*(i-1)+1:long_trame*i);
    
    % Détection d'intensité
    intensite = mean(abs(trame));
    log_intensite(i)=intensite;
    
    if(intensite>seuil)
        
        n_trames_son = n_trames_son + 1;
        
        % On ignore les premières trames, pour n'obtenir que le régime
        % transitoire
        if(n_trames_son>n_trames_to_skip && ((n_trames_son-n_trames_to_skip)<=n_trames_to_keep))

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
            
            % Si c'est les premiers peaks, initialiser le vecteur de log
            % pour des raisons d'efficacité. On ne garde que trois sets de
            % peaks
            n_trame = n_trames_son-n_trames_to_skip;
            if((n_trame)==1)
               log_peaks=zeros(n_trames_to_keep,n_peaks); 
            end
            
            log_peaks(n_trame,:)=peaks;
            
            % Détection de la note
            FFT_trame = fft(hanning(long_trame)'.*trame);
            mag_FFT_trame =  abs(FFT_trame);
            [amax,imax] = max(mag_FFT_trame);
            freq_trames(i) = (imax-1)/(long_trame)*Fe;
            
            
            figure(1)
            plot(autocorr_trame);
            hold on
            plot(peaks,autocorr_trame(peaks),'ro')
            
            peaks;
        elseif(((n_trames_son-n_trames_to_skip)>n_trames_to_keep))
           % Détection de la périodicité
           periodique=1;
            for k=1:length(log_peaks)
                if(abs(log_peaks(1,k)-log_peaks(2,k))>deviation_ecart_peak_max)
                    periodique=0;
                end
            end
           
           
           
           periodique=1;
            % Filtrage de la périodicité des peaks
            if(periodique==1)
                trame_periodique(i-n_trames_to_keep:i) = ones(1,n_trames_to_keep+1);
            end
            log_peaks
            
        end
        
    end
end

figure(2)
area(trame_periodique)

figure(3)
%plot(trame_periodique.*freq_trames)
plot(freq_trames)

figure(4)
plot(log_intensite)
