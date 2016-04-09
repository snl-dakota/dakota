% modified mean, std. dev (of elemenary effects)
MS(1,:,:)=[ 2.9498e+00   1.8505e+00 
	    2.4340e+00   1.4531e+00 
	    2.2076e+00   8.0594e-01 
	    5.1440e-01   3.3147e-01 
	    0.0000e+00   0.0000e+00 
	    0.0000e+00   0.0000e+00 
	     0.0000e+00   0.0000e+00  ];
MS(2,:,:)=[ 2.3546e+04   2.1084e+04 
 2.6249e+04   1.5812e+04 
 0.0000e+00   0.0000e+00 
 0.0000e+00   0.0000e+00 
 0.0000e+00   0.0000e+00 
 1.1218e+04   1.1506e+04 
	     7.3190e+03   6.3244e+03  ];

MS(3,:,:) = [  1.7906e-02   3.5581e-02 
  1.5680e-02   2.2762e-02 
  1.7198e-02   2.0252e-02 
  0.0000e+00   0.0000e+00 
  1.6842e-03   1.7291e-03 
  1.0018e-02   1.9822e-02 
		3.9993e-03   1.0587e-02 ];

vars={'w', 't', 'L', 'E', '\rho',  'X', 'Y', };
resp={'mass', 'stress', 'displ'};


vshifts = [  1.9 1.5 0.9, 0.4 .0005, .0005, .003
            2.4e4 2.7e4, -50, 50, 50, 1.2e4, 8e3
	      1.8e-2 1.6e-2 1.8e-2, 0, 1.7e-3, 1.1e-4, 4.0e-3
	    ];
hshifts = [  3.0, 2.5, 2.3, 0.6, 0.10, 0.01, .10
	      2.2e4, 1.6e4, 20, 500, 0, 1.2e4, 7e3  
	      3.6e-2 2.3e-2, 2.1e-2, 0, 1.8e-3, 2.0e-2 1.1e-2
	    ];

vshifts = zeros(3,7);
hshifts = zeros(3,7);

%vshifts(2,6) = -0.5e4
  hshifts(3,4) = 0.0005
  hshifts(3,5) = 0.0005

  hshifts(2,4) = 0.2e4
  vshifts(2,3) = 0.2e4

  vshifts(1,6)=0.1
  hshifts(1,7)=0.1

shift=10;

figure(3)

mywidth=9.0;
myheight=3.0;
set(3, 'PaperPosition', [0   0    mywidth    myheight])
set(3, 'PaperSize', [mywidth myheight])

for sp=1:3,
	 sp_axes = subplot(1,3,sp);
  axis square
	 M=squeeze(MS(sp,:,:));
     hold on
     for i=1:7,
       plot(M(i,1),M(i,2),'.r','MarkerSize',12);
     end
     hold off
%     for i=8:10,
%       plot(M(i,1),M(i,2),'sb','MarkerSize',5);
%     end
%     for i=11:20,
%       plot(M(i,1),M(i,2),'xk','MarkerSize',5);
%     end
     xlabel('main effect (\mu^*)','FontSize',12,'FontWeight','bold');
if (sp == 1)
     ylabel('interaction effect (\sigma)','FontSize',12,'FontWeight','bold');
end
     for i=1:7,
	     text(1.05*M(i,1)+hshifts(sp,i), 1.05*M(i,2)+vshifts(sp,i), vars{i},'FontWeight','bold','FontSize',12)
     end
%     text(5,15,'11-20','FontWeight','bold')  
%     ax = get(3,'CurrentAxes');
     set(sp_axes,'FontSize',12);

title(resp{sp}, 'FontSize',16,'FontWeight','bold')

     %print('-depsc2','cantilever_morris.eps')
     %print('-dpng','cantilever_morris.png')
     print('-dpdf','cantilever_morris')
end
break
