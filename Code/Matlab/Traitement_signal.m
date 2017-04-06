close all
clear all
clc


%% Fichiers audio

% 
%  Fe = 8000;
%  Long_sinus = Fe*10; % 10 secondes
%  n = 1:Long_sinus;
%  freq_norm = 2*pi*n/Fe*250;
%  note_audio = sin(freq_norm)';

% [note_audio,Fe] = audioread('Bruits/Enregistrement_8.wav');
%
%
% [note_audio,Fe] = audioread('Notes/Do_8.wav');
% [note_audio,Fe] = audioread('Notes/Do#_8.wav');
% [note_audio,Fe] = audioread('Notes/Re_8.wav');
% [note_audio,Fe] = audioread('Notes/Re#_8.wav');
% [note_audio,Fe] = audioread('Notes/Mi_8.wav');
% [note_audio,Fe] = audioread('Notes/Fa_8.wav');
% [note_audio,Fe] = audioread('Notes/Fa#_8.wav');
% [note_audio,Fe] = audioread('Notes/Sol_8.wav');
% [note_audio,Fe] = audioread('Notes/Sol#_8.wav');
% [note_audio,Fe] = audioread('Notes/La_8.wav');
% [note_audio,Fe] = audioread('Notes/La#_8.wav');
% [note_audio,Fe] = audioread('Notes/Si_8.wav');
%
% [note_audio,Fe] = audioread('Gammes/Gamme_majeur_Do_8.wav');
% [note_audio,Fe] = audioread('Accords/Arpege_mongol.wav');

% [note_audio,Fe] = audioread('Accords/C.wav');
% [note_audio,Fe] = audioread('Accords/C+G.wav');
% [note_audio,Fe] = audioread('Accords/D.wav');
 [note_audio,Fe] = audioread('Accords/Dmin.wav'); % Fonctionne pas
% [note_audio,Fe] = audioread('Accords/Ginv.wav');

figure()
plot(note_audio)
title('Note audio')
xlabel('No d''échantillon')
ylabel('Intensité')

%% Algorithme

% Matlab Debug
plot_FFT=0;
plot_FFT_couleur = 1;
n_trames_fft_plot=3;
conversion_en_notes=0; % TODO

% Constantes
SEUIL_INTENSITE = 0.02;
DECALAGE_AUTOCORR = 64;
LONG_TRAME = 1024; % À NE PAS CHANGER. Donne la précision fréquentielle
DEVIATION_ECART_PEAK_MAX = 6; % Tolérance pour la détection de périodicité
N_TRAMES_TO_SKIP = 1; % Permet d'ignorer la phase transitoire pour l'analyse
N_TRAMES_TO_KEEP = 2; % À NE PAS CHANGER. Seulement pour la détec. périod.
N_ECH_MOY_INTENSITE = 64; % Nombre d'échantillons pour détection intensité
RATIO_PEAK_FFT = 16; % Ratio entre hauteur peak accepté et intensité trame

% Variables
n_trames = ceil(length(note_audio)/LONG_TRAME);
trame = zeros(1,LONG_TRAME);
freq_trames=zeros(3,n_trames);
autocorr_trame = zeros(1,2*DECALAGE_AUTOCORR+1);

% VARIABLES DEBUG
log_intensite = zeros(1,n_trames);
log_periodique = zeros(1,n_trames);

oscillation_dB = 1;
% Conception du filtre passe-haut
f_coupure1 = 250/(Fe/2);
[A,B,C,D] = cheby1(10,oscillation_dB , f_coupure1,'high');
[sos1,gain_global1] = ss2sos(A,B,C,D, 'up', 'inf');
[b1,a1] = sos2tf(sos1, gain_global1);

% Conception du filtre passe-bas
freq_max_gamme = 520;
f_coupure2 = freq_max_gamme/(Fe/2);
[A,B,C,D] = cheby1(10, oscillation_dB, f_coupure2,'low');
[sos2,gain_global2] = ss2sos(A,B,C,D, 'up', 'inf');
[b2,a2] = sos2tf(sos2, gain_global2);

% Analyse du son par trames
n_trames_son = 0;
for n_trame = 1:n_trames
    
    % Formation de la trame
    if(n_trame==n_trames)
        n_derniere_trame = length(note_audio)-(n_trames-1)*LONG_TRAME;
        trame = [note_audio(LONG_TRAME*(n_trames-1)+1:LONG_TRAME*(n_trames-1)+n_derniere_trame)',zeros(1,LONG_TRAME-n_derniere_trame)];
    else
        trame  = note_audio(LONG_TRAME*(n_trame-1)+1:LONG_TRAME*n_trame)';
    end


    
    % Détection d'intensité
    intensite = mean(abs(trame(1:N_ECH_MOY_INTENSITE)));
    log_intensite(n_trame)=intensite;
    
    
    if(intensite>SEUIL_INTENSITE) % Si c'est un son
           
    % Filtrage de la trame
    % Passe-haut
    trame=filter(b1,a1,trame);
    % Passe-bas
    trame=filter(b2,a2,trame);
        
        n_trames_son = n_trames_son + 1;
        if(n_trames_son>N_TRAMES_TO_SKIP)
            n_trame_analyse = n_trames_son-N_TRAMES_TO_SKIP;
                       
            if(n_trame_analyse<=N_TRAMES_TO_KEEP)
                
                % Padding
                trame_pad = [zeros(1,DECALAGE_AUTOCORR), trame, zeros(1,DECALAGE_AUTOCORR)];
                % Autocorrélation
                somme = 0;
                for v = -DECALAGE_AUTOCORR:DECALAGE_AUTOCORR
                    for n = 1:(LONG_TRAME - DECALAGE_AUTOCORR)
                        somme = somme + trame_pad(n+DECALAGE_AUTOCORR)*trame_pad(n+v+DECALAGE_AUTOCORR);
                    end
                    autocorr_trame(v+DECALAGE_AUTOCORR+1) = somme;
                end
                
                
                % Détection de peaks
                % TODO : Ne garder que les plus grands peaks (fondamentale)
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
                log_peaks=zeros(N_TRAMES_TO_KEEP,n_peaks);
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
                        if(abs(log_peaks(1,k)-log_peaks(2,k))>DEVIATION_ECART_PEAK_MAX)
                            periodique=0;
                        end
                    end
                    log_periodique(n_trame-1:n_trame)=periodique*ones(1,2);
                end
            else
                log_periodique(n_trame)=periodique;
            end
            
            
            % Détection des notes
            % TODO : Ne faire l'analyse fréquentielle qu'au début de la
            % note
            if(periodique)
                FFT_trame = fft(hanning(LONG_TRAME)'.*trame);
                mag_FFT_trame =  abs(FFT_trame);
                % Détection de peaks
                % Étape 1 : Trouver l'indice i_fin de fin de gamme
                i_fin = floor(freq_max_gamme*LONG_TRAME/Fe+1);
                % Étape 2 : Chercher les peaks jusqu'à l'indice i
                n_peaks_FFT = 0;
                peaks_FFT = ones(1,3);
                seuil_peak_FFT = log_intensite(n_trame)*RATIO_PEAK_FFT;
                for k = 3:(i_fin-2)
                    if(mag_FFT_trame(k)>seuil_peak_FFT)
                        if(mag_FFT_trame(k)>mag_FFT_trame(k-1) && mag_FFT_trame(k)>mag_FFT_trame(k-2))
                            if(mag_FFT_trame(k)>mag_FFT_trame(k+1) && mag_FFT_trame(k)>mag_FFT_trame(k+2))
                                n_peaks_FFT=n_peaks_FFT+1;
                                peaks_FFT (1,n_peaks_FFT)= k;
                            end
                        end
                    end
                end
                freq_trames(:,n_trame) = (peaks_FFT-1)/(LONG_TRAME)*Fe;
                if(n_trame_analyse==2)
                    freq_trames(n_trame-1) = freq_trames(n_trame);
                end
            end
            
            figure(2)
            plot(autocorr_trame);
            hold on
            plot(peaks,autocorr_trame(peaks),'ro')
            title('Autocorrélation et détection de peaks')
            
            if(periodique && plot_FFT && n_trame_analyse<=n_trames_fft_plot)
                figure()
                plot(mag_FFT_trame(1:40));
                title(['FFT de la trame ' num2str(n_trame_analyse)]);
                xlabel('Indice k')
                ylabel('Amplitude')
            end
            
            if(periodique && plot_FFT_couleur)
                figure(30)
                title('FFT des trames périodiques');
                xlabel('Indice k')
                ylabel('Amplitude')
                hold on
                color = 1 - 4*intensite;
                plot(mag_FFT_trame(20:80),'Color',[color,color,color]);
            end
            
        end
    else % si l'intensité est sous le seuil
        n_trames_son=0;
    end
end

figure()
area(log_periodique)
title('Périodicité des trames')
xlabel('Numéro de trame')
ylabel('Périodique')

figure()

hold on
plot(freq_trames(1,:))
plot(freq_trames(2,:))
plot(freq_trames(3,:))
title('Fréquence des trames')
xlabel('Numéro de trame')
ylabel('Fréquence détectée (Hz)')

figure()
x=1:length(log_intensite);
x=x*LONG_TRAME/Fe;
plot(x,log_intensite)
title(['Intensité des trames, moyenne sur ' num2str(N_ECH_MOY_INTENSITE) ' échantillons'])
xlabel('Temps (s)')
ylabel('Intensité')
axis([0 x(end) 0 1])




