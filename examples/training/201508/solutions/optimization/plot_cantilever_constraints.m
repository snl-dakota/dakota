ax = [0.5, 4.0, 0.5, 4.0];

N=65;
w=repmat(((0:N-1)/(N-1))'*(ax(2)-ax(1))+ax(1),1,N);
t=repmat(((0:N-1)/(N-1)) *(ax(4)-ax(3))+ax(3),N,1);

rho = 500.0;
L = 5.0;
Y = 500.0;
X = 350.0;
E = 2.9e7;

alpha = 600*Y;
beta = 600*X;

dakotared  =[0.83 0    0   ]; %inputs
dakotagreen=[0    0.8  0   ]; %constraints


figure(1)

hold on

mass = rho * w .* t * L/(12^3);
[c,h]=contour(w,t,mass,[2:2:20],'linestyle','-', 'linewidth',2);
clabel(c,h,'FontSize',14,'FontWeight','bold');

axis equal;
axis(ax(1:4));
set(gca,'Fontsize',16);
xlabel('w','Fontsize',18);
ylabel('t','Fontsize',18); 

stress = 600*Y./(w.*t.^2) + 600*X./(w.^2.*t);
contour(w,t,stress,[1.0e5 1.0e5],'linecolor',dakotared,'linewidth',4, 'linestyle','--')

s1 = (Y./(t.^2)).^2;
s2 = (X./(w.^2)).^2;
displ = 4*L^3/E./w./t.*sqrt(s1 + s2);
contour(w,t,displ,[0.001 0.001],'linecolor',dakotagreen,'linewidth',4, 'linestyle','--')
 
  
hold off

h=legend('mass','stress = 1.0e5','displacement = 0.001', ...
         'location','northeast');
set(h,'fontsize',16);

print('-dpng','cantilever_contours');        
