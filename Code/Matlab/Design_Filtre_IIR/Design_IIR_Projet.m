% calculs IIR projet S5
% gabriel guilmain

clc;
clear all;
close all;

fe = 8000;
oscillation_dB = 2;

% frequences pour loctave dinteret (5eme)
% B4: 493,88 Hz (a filtrer)
% C5: 523,25 Hz (on conserve cette frequence)
% C6: 1046,50 Hz (on conserve cette frequence)
% C#6: 1108,93 Hz (a filtrer)

%% 1ER FILTRE: passe haut 

% conception du filtre
f_coupure1 = 515/(fe/2);
[A B C D] = cheby1(10,oscillation_dB , f_coupure1,'high');
[sos1 gain_global1] = ss2sos(A,B,C,D, 'up', 'inf');

[b1 a1] = sos2tf(sos1, gain_global1);
figure
zplane(b1,a1);
title('plan en z du filtre passe haut')

[H,W] = freqz(b1,a1,10000);

% reponse en frequence et verif des specs
figure
plot(W,20*log10(abs(H)))
hold on
plot((2*pi*523.25/(fe))*ones(size(-50:0.01:10)),-50:0.01:10)
hold on
plot((2*pi*493.88/(fe))*ones(size(-50:0.01:10)),-50:0.01:10)
title('conception du filtre passe haut (reponse en amplitude)')

% test filtrage et Génération de sinus
n1=1:1:256;
f1=600; % frequence a rentrer dans le systeme pour observer dans domaine temporel
fe=8000;
x1 = sin(2*pi*f1*n1/(fe));
figure
plot(1:length(x1),x1)
hold on

y1 = filter(b1,a1,x1);
plot(1:length(y1),y1)
title('test du filtre passe haut (domaine temporel)')

% enregistrement des coeffs
dlmwrite('coeff_filtre1.txt', sos1);  
dlmwrite('coeff_filtre1.txt',gain_global1,'-append','delimiter',' ','roffset',1)

%% 2EME FILTRE: passe bas 
f_coupure2 = 1060/(fe/2);
[A B C D] = cheby1(10, oscillation_dB, f_coupure2,'low');
[sos2 gain_global2] = ss2sos(A,B,C,D, 'up', 'inf');

[b2 a2] = sos2tf(sos2, gain_global2);
figure
zplane(b2,a2);
title('plan en z du filtre passe bas')

[H2,W2] = freqz(b2,a2,10000);

% reponse en frequence et verif des specs
figure
plot(W2,20*log10(abs(H2)))
hold on
plot((2*pi*1108.93/(fe))*ones(size(-50:0.01:10)),-50:0.01:10)
hold on
plot((2*pi*1046.50/(fe))*ones(size(-50:0.01:10)),-50:0.01:10)
title('conception du filtre passe bas (reponse en amplitude)')

% cascade des deux filtre pour obtenir le passe bande
figure
plot(W2,20*log10(abs(H.*H2)))
title('reponse en amplitude des deux filtre en cascade')

% test filtrage et Génération de sinus
n2=1:1:256;
f2=1038;  % frequence a rentrer dans le systeme pour observer dans domaine temporel
fe=8000;
x2= sin(2*pi*f2*n2/(fe));
figure
plot(1:length(x2),x2)
hold on

y2 = filter(b2,a2,x2);
plot(1:length(y2),y2)
title('test du filtre passe bas (domaine temporel)')

% enregistrement des coeffs
dlmwrite('coeff_filtre2.txt', sos2);  
dlmwrite('coeff_filtre2.txt',gain_global2,'-append','delimiter',' ','roffset',1)


