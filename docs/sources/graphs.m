data= [10, 1.563,  0.479, 1.531, 0.461,  3.168,  0.099,  1.357, 0.340;
       50, 2.629,  0.517, 1.554, 0.484,  3.166,  0.102,  1.448, 0.430;
      100, 1.539,  0.467, 1.654, 0.495,  5.192,  0.104,  1.610, 0.570;
     1000, 2.659,  0.544, 1.726, 0.581,  1.264,  0.247,  6.234, 4.800;
    10000, 47.178, 1.278, 5.542, 1.428, 18.718, 13.194, 10.254, 8.540];

data_2 = [    10, 1.350, 0.17, 1.339, 0.32;
              50, 1.278, 0.15, 1.328, 0.31;
             100, 1.260, 0.15, 1.441, 0.42;
            1000, 1.305, 0.19, 1.340, 0.33;
           10000, 1.508, 0.35, 2.115, 0.56];
%%
h=figure();
subplot(1,2,1)
loglog(data(:,1)',data(:,2)','-o');
hold on
grid on
loglog(data(:,1)',data(:,4)','-o');
loglog(data(:,1)',data(:,6)','-o');
legend('Iterativo','Chiamata a funzione', 'Ricorsivo','Location','NorthWest');
xlabel("Numero di messaggi");
ylabel("Tempo (s)");
title("Tempo totale");
hold off
subplot(1,2,2);
loglog(data(:,1)',data(:,3)','-o');
hold on
grid on
loglog(data(:,1)',data(:,5)','-o');
loglog(data(:,1)',data(:,7)','-o');
xlabel("Numero di messaggi");
ylabel("Tempo (s)");
title("Tempo netto");
legend('Iterativo','Chiamata a funzione', 'Ricorsivo','Location', 'NorthWest');
hold off

set(h,'Units','Inches');
pos = get(h,'Position');
set(h,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])
print(h,'performance-graph','-dpdf','-r0')
%%
j=figure();
subplot(2,2,1)
loglog(data(:,1)',data(:,4)','-o');
hold on
grid on
loglog(data(:,1)',data(:,8)','-o');
legend('PASTEL','Python','Location','NorthWest');
xlabel("Numero di messaggi");
ylabel("Tempo (s)");
title("Tempo totale, connessioni separate");
hold off

subplot(2,2,2);
loglog(data(:,1)',data(:,5)','-o');
hold on
grid on
loglog(data(:,1)',data(:,9)','-o');
xlabel("Numero di messaggi");
ylabel("Tempo (s)");
title("Tempo netto, connessioni separate");
legend('PASTEL','Python','Location','NorthWest');
hold off

subplot(2,2,3)
loglog(data_2(:,1)',data_2(:,2)','-o');
hold on
grid on
loglog(data_2(:,1)',data_2(:,4)','-o');
legend('PASTEL','Python','Location','NorthWest');
xlabel("Numero di messaggi");
ylabel("Tempo (s)");
title("Tempo totale, connessione singola");
hold off

subplot(2,2,4);
loglog(data(:,1)',data(:,3)','-o');
hold on
grid on
loglog(data_2(:,1)',data_2(:,5)','-o');
xlabel("Numero di messaggi");
ylabel("Tempo (s)");
title("Tempo netto, connessione singola");
legend('PASTEL','Python','Location','NorthWest');
hold off



set(j,'Units','Inches');
pos = get(j,'Position');
set(j,'PaperPositionMode','Auto','PaperUnits','Inches','PaperSize',[pos(3), pos(4)])
print(j,'pvp','-dpdf','-r0')
