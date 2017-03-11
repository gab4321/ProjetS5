close all
clear all
clc



%[Freq800,Fe] = audioread('800.wav');
%Freq800 = Freq800(1:1000,1);

%[LA,FeLA] = audioread('Enregistrement.m4a');


% [LA,FeLA] = audioread('DO5.wav'); %Pas correct
% [LA,FeLA] = audioread('DO#.wav');
% [LA,FeLA] = audioread('Re.wav');
% [LA,FeLA] = audioread('Re#.wav');
% [LA,FeLA] = audioread('Mi.wav'); % Pas correct
% [LA,FeLA] = audioread('Fa.wav');
% [LA,FeLA] = audioread('_Fa#.wav');
% [LA,FeLA] = audioread('_sol.wav'); % Pas correct
% [LA,FeLA] = audioread('_sol#.wav');
% [LA,FeLA] = audioread('_LA.wav');
% [LA,FeLA] = audioread('LA#.wav'); % Fonctionne, mais peaks en bas
% [LA,FeLA] = audioread('Si.wav');

 [LA,FeLA] = audioread('Gamme majeur Do.wav'); % Combination d'aberrations



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
n_trames_son = 0;
n_trames_to_skip = 3;
% Analyse du son par trames
for i = 1:n_trames
    
    % Formation de la trame
    trame  = LA(long_trame*(i-1)+1:long_trame*i);
    
    % Détection d'intensité
    intensite = mean(abs(trame));
    log_intensite(i)=intensite;
    
    if(intensite>seuil)
        
        n_trames_son = n_trames_son + 1;
        
        
        % On ignore les premières trames, pour n'obtenir que le régime
        % transitoire
        if(n_trames_son>=n_trames_to_skip)

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
            
            
            % Détection de peaks
            % TODO : S'assurer que la valeur absolue des peaks dépasse un
            % certain seuil à déterminer
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
            
            % Détection de la périodicité des peaks
            periodique = 1;
            for k=2:(length(peaks)-1)
                ecart = abs((peaks(k)-peaks(k-1))-(peaks(k+1)-peaks(k)));
                if(ecart>deviation_peak_max)
                    periodique = 0;
                end
            end
            trame_periodique(i) = periodique;
            
            % Détection de la note
            
            
            figure(1)
            plot(autocorr_trame);
            hold on
            plot(peaks,autocorr_trame(peaks),'ro')
            
            peaks
        end
    end
end

figure(2)
area(trame_periodique)
