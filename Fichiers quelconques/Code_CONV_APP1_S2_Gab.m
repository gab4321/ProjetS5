%determination de la constante de temps du deuxieme filtre RC de L'APP1
%calcul de la convolution de la reponse impulsionnelle et du signal
%d'entree
%affichage du signal d'entree et du signal de sortie
clear all;
clc;

fE = 0;
f = 15 + fE;
tho = [0.015]; %tableau pour essayer plusieurs constantes de temps
T = 1/f;
dt = T/10000;
t = 0:dt:1;
A = 2.6854;

%sinus de 5Hz
AE = ((1/sqrt(1+(fE/100)^2))*((1/100)/sqrt(1+(fE/100)^2)))*3;
z = abs(AE*sin(2*pi*fE*t));

figure;

%plot(t,z,'y');
hold on

x = abs(A*sin(2*pi*f*t));
plot(t,x,'r');

for i = 1:length(tho)
    h = (exp(-t/tho(i)))/tho(i);
    y = conv(x,h)*dt;
    plot(t,y(1:length(t)));
    plot(t,h,'g');
end

title('convolution pour le 2eme reseau RC ');
legend('x(t)','y(t)','h(t)');
axis([0 8*T -1 4 ]);
xlabel('temps(s)');
ylabel('amplitude(V)');
hold off