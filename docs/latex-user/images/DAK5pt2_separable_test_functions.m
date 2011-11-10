n1d=33;
x1=repmat((0:n1d-1)'/(n1d-1)*4-2,1,n1d);
x=x1(:);
x2=x1';
x(:,2)=x2(:);
yada1_sm=exp(-(x(:,1)-1).^2)+exp(-0.8*(x(:,1)+1).^2);
yada1_h=yada1_sm-0.05*sin(8*(x(:,1)+0.1));
yada2_sm=exp(-(x(:,2)-1).^2)+exp(-0.8*(x(:,2)+1).^2);
yada2_h=yada2_sm-0.05*sin(8*(x(:,2)+0.1));
herbie=-reshape(yada1_h.*yada2_h,n1d,n1d); %herbie
smooth_herbie=-reshape(yada1_sm.*yada2_sm,n1d,n1d); %herbie
max_hsmh=1.2;
min_hsmh=-0.2;

fig=figure;
WX=6;
WY=3;
set(fig,'units','inches','paperunits','inches','paperposition',[(8.5-WX)/2 (11-WY)/2 WX WY],'PaperOrientation','portrait')
pos=get(fig,'position');
pos2(1)=pos(1); pos2(3)=WX; pos2(4)=WY; pos2(2)=pos(2)+pos(4)-pos2(4);
set(fig,'position',pos2)
ax0=[0.33 2.97]/WX;
ay0=0.4/WY;
awx=2.25/WX;
awy=2.25/WY;
caxis_lim=[-1.2 0];

hax_herbie=axes('position',[ax0(1) ay0 awx awy]); 
surf(x1,x2,herbie); shading interp; 
caxis(caxis_lim); 
hold on; plot3(x1,x2,herbie,'-k',x1',x2',herbie','-k'); hold off; 
set(hax_herbie,'zdir','reverse');
axis([-2 2 -2 2 caxis_lim]);
xlabel('x1','fontsize',14);
ylabel('x2','fontsize',14);
title('2D "herbie"','fontsize',16);

hax_smooth_herbie=axes('position',[ax0(2) ay0 awx awy]); 
surf(x1,x2,smooth_herbie); shading interp; 
caxis(caxis_lim); 
hold on; plot3(x1,x2,smooth_herbie,'-k',x1',x2',smooth_herbie','-k'); hold off; 
set(hax_smooth_herbie,'zdir','reverse');
axis([-2 2 -2 2 caxis_lim]);
xlabel('x1','fontsize',14);
ylabel('x2','fontsize',14);
title('2D "smooth\_herbie"','fontsize',16);

ColBarAxPos=[ax0(1) ay0 0.99-ax0(1) awy];
ColBarAxes=axes('position',ColBarAxPos,'visible','off','tag','col_bar_axis');
caxis(caxis_lim);
colorbar vert;

print -dpng DAK5pt2_2D__herbie__smooth_herbie
print -depsc DAK5pt2_2D__herbie__smooth_herbie

n2d=n1d^2;
i=1:5; 
yada1_shu=sum(repmat(i,n2d,1).*cos(x(:,1)*(i+1)+repmat(i,n2d,1)),2);
yada2_shu=sum(repmat(i,n2d,1).*cos(x(:,2)*(i+1)+repmat(i,n2d,1)),2);
shubert=reshape(yada1_shu.*yada2_shu,n1d,n1d);

fig_shu=figure;

WX=3.4;
WY=3;
set(fig_shu,'units','inches','paperunits','inches','paperposition',[(8.5-WX)/2 (11-WY)/2 WX WY],'PaperOrientation','portrait')
pos=get(fig_shu,'position');
pos2(1)=pos(1); pos2(3)=WX; pos2(4)=WY; pos2(2)=pos(2)+pos(4)-pos2(4);
set(fig_shu,'position',pos2)
ax0=0.33/WX;
ay0=0.4/WY;
awx=2.25/WX;
awy=2.25/WY;


hax_shubert=axes('position',[ax0 ay0 awx awy]);
surf(x1,x2,shubert); shading interp; 
hold on; plot3(x1,x2,shubert,'-k',x1',x2',shubert','-k'); hold off; 
%axis([-2 2 -2 2 caxis_lim]);
shu_ax=axis;
xlabel('x1','fontsize',14);
ylabel('x2','fontsize',14);
title('2D "shubert"','fontsize',16);

ColBarAxPos=[ax0 ay0 0.97-ax0 awy];
ColBarAxes=axes('position',ColBarAxPos,'visible','off','tag','col_bar_axis');
caxis(shu_ax(5:6));
colorbar vert;

print -dpng DAK5pt2_2D_shubert
print -depsc DAK5pt2_2D_shubert
