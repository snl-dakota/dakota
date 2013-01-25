function matlab_manual_image_generator(varargin)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%start matlab in the .../docs/latex-user directory and then run this
%function. Usage...
%
% matlab_tutorial_image_generator() regenerates all .png plots in the 
%       tutorial chapter of the dakota users manual (you still need to run
%       gimp to autocrop the images)
%
% matlab_tutorial_image_generator(imagefilename) regenerates only the .png
%       plot whose name is stored in the string variable imagefilename (you
%       still need to run gimp on this image to autocrop it)
%
% matlab_tutorial_image_generator(imagefilename1,imagefilename2,...,
%       imagefilenameN) regenerates only the .png plots whose names are
%       contained in the N stringvariables imagefilename1,imagefilename2,
%       ...,imagefilenameN (you still need to run gimp on these image to 
%       autocrop them)
%
% after creating these .png images, this functions outputs to the matlab
% command line the invocation of gimp (gnu image manipulation program) that
% will open all of the just produced .png files at once, and provides 
% instructions for using gimp to autocrop the images and how to then save
% the modified images.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    path_to_examples='../../examples/users/';
    %path_to_save_images='develop_images/'; %I used this for development
    path_to_save_images='images/'; %use this for production
    ax_rosen_full=[-2 2 -2 2 0 4000];
    ax_rosen_zoom=[-0.5 0.5 -0.5 0.5 0 4000];
    ax_rosen_sc_lognorm=[0 30 .5 4];
    ax_textbook_full=[-3 4 -3 4];
    ax_textbook_zoom=[-1 1 -1 1];
    
    %dakota doesn't use maximally bright colors in it's outputs they look
    %closer to the following, note that these are approximate (dakota green 
    %was hard to judge since only example is a thin line in the textbook 
    %problem), these colors are hardcoded into the subfunctions below,
    %could modify to pass them in as arguments but why bother
    %dakotared  =[0.83 0    0   ]; %inputs
    %dakotagreen=[0    0.8  0   ]; %constraints
    %dakotablue =[0    0    0.87]; %outputs
    
    if(nargin==0)
        %this is a complete listing of the files it can produce, and it
        %does produce all of them when this function is passed no arguments
        output_files={...
            'rosen_3d_surf.png',...
            'rosen_2d_surf.png',...
            'textbook_contours.png',...
            'textbook_closeup.png',...
            'rosen_2d_pts.png',...
            'rosen_vect_pts.png',...
            'rosen_grad_opt_pts.png',...
            'textbook_history.png',...
            'rosen_ps_opt_pts.png',...
            'rosen_ps_opt_pts2.png',...
            'rosen_ea_init.png',...
            'rosen_ea_final.png',...
            'dakota_mogatest1_pareto_front.png',...
            'rosen_nond_pts.png',...
            'rosen_pce_pts.png',...
            'rosen_sc_pts.png',...
            'dakota_mogatest2_pareto_front.png',...
            'dakota_mogatest3_pareto_set.png',...
            'dakota_mogatest3_pareto_front.png'};
        Nout=length(output_files);
    else
        %only regenerate the images whose filenames were passed into this
        %function as string variables (it accepts an arbitrary number of
        %inputs)
        Nout=nargin;
        output_files= varargin;
    end
            
    gimp_command_string='gimp'; %start the gimp command string, below we 
    %will append the filenames of all images that we regenerate
    for iout=1:Nout
        switch(char(output_files{iout}))
            case 'rosen_3d_surf.png'
                imagename=plot_rosenbrock_3D(ax_rosen_full,...
                    path_to_save_images);
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'rosen_2d_surf.png'
                fig=figure;
                iflegend=1; %if plot legend
                imagename=plot_rosenbrock_contour(fig,ax_rosen_full,...
                    iflegend,path_to_save_images);
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'textbook_contours.png'
                fig=figure; 
                ifcon=1; %if plot constraints
                iflegend=1; %if plot a legend
                imagename=plot_textbook_contour(fig,ax_textbook_full,...
                    ifcon,iflegend,path_to_save_images,...
                    char(output_files{iout}));
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                      imagename);
            case 'textbook_closeup.png'
                fig=figure; 
                ifcon=1; %if plot constraints
                iflegend=1; %if plot a legend
                imagename=plot_textbook_contour(fig,ax_textbook_zoom,...
                    ifcon,iflegend,path_to_save_images,...
                    char(output_files{iout}));
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'rosen_2d_pts.png'
                imagename=plot_rosenbrock_2d_param_study(...
                    path_to_examples,ax_rosen_full,...
                    path_to_save_images);
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'rosen_vect_pts.png'
                imagename=plot_rosenbrock_vector_param_study(...
                    path_to_examples,ax_rosen_full,...
                    path_to_save_images);
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'rosen_grad_opt_pts.png'
                imagename=plot_rosenbrock_grad_opt(...
                    path_to_examples,ax_rosen_full,...
                    path_to_save_images);
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);                
            case 'textbook_history.png'
                imagename=plot_textbook_constrained_opt_example(...
                    path_to_examples,ax_textbook_full,...
                    path_to_save_images);
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'rosen_ps_opt_pts.png'
                fname='rosen_ps_opt_pts.png';
                imagename=plot_rosenbrock_ps_opt(...
                    path_to_examples,ax_rosen_full,...
                    path_to_save_images,char(output_files{iout}));
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'rosen_ps_opt_pts2.png'
                imagename=plot_rosenbrock_ps_opt(...
                    path_to_examples,ax_rosen_zoom,...
                    path_to_save_images,char(output_files{iout}));
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'rosen_ea_init.png' 
                imagename=plot_rosenbrock_ea_opt(...
                    path_to_examples,ax_rosen_full,...
                    path_to_save_images,char(output_files{iout}));
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'rosen_ea_final.png' 
                imagename=plot_rosenbrock_ea_opt(...
                    path_to_examples,ax_rosen_full,...
                    path_to_save_images,char(output_files{iout}));
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'dakota_mogatest1_pareto_front.png'
                imagename=plot_mogatest1(path_to_examples,...
                    path_to_save_images);
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'rosen_nond_pts.png'
                imagename=plot_rosenbrock_nond(...
                    path_to_examples,ax_rosen_full,...
                    path_to_save_images);
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'rosen_pce_pts.png'
                imagename=plot_rosenbrock_pce(...
                    path_to_examples,ax_rosen_full,...
                    path_to_save_images);
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'rosen_sc_pts.png'
                imagename=plot_rosenbrock_sc(...
                    path_to_examples,ax_rosen_sc_lognorm,...
                    path_to_save_images);
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'dakota_mogatest2_pareto_front.png'
                imagename=plot_mogatest2(path_to_examples,...
                    path_to_save_images);
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'dakota_mogatest3_pareto_set.png'
                imagename=plot_mogatest3(path_to_examples,...
                    path_to_save_images,char(output_files{iout}));
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            case 'dakota_mogatest3_pareto_front.png'
                imagename=plot_mogatest3(path_to_examples,...
                    path_to_save_images,char(output_files{iout}));
                gimp_command_string=sprintf('%s %s',gimp_command_string,...
                    imagename);
            otherwise
                error('unknown output file "%s"',char(output_files{iout}));
        end
    end
    gimp_command_string=sprintf('%s &',gimp_command_string);
    
    %print instructions for autocropping the image files in gimp for the
    %user... not the string contained in gimp command string will open
    %every image this function just regenerated in one invocation (i.e. it
    %saves the user time and decreases the likelihood of an image being
    %missed)
    disp(sprintf('\nInstructions, do the following:\ncd %s',...
        path_to_save_images));
    disp(gimp_command_string);
    disp(sprintf('Within gimp do a "image->autocrop image" followed %s',...
        'by a "file->save" for each image'));
    disp('Also doing a "file->save as" to ".eps" format is optional');    
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%the individual functions below this line only create image files if they
%are called with the "path_to_save_images" argument, i.e. you can preview
%images without writing them to file by omitting them "path_to_save_images"
%and later arguments when calling them.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_rosenbrock_3D(ax,path_to_save_images)
    N=129;
    x1=repmat(((0:N-1)/(N-1))'*(ax(2)-ax(1))+ax(1),1,N);
    x2=repmat(((0:N-1)/(N-1)) *(ax(4)-ax(3))+ax(3),N,1);
    f=100*(x2-x1.^2).^2+(1-x1).^2;
    fig=figure;
    colormap('white'); 
    surf(x1,x2,f);
    shading interp;
    hold on;
    plot3(x1(:,1:8:N) ,x2(:,1:8:N) ,f(:,1:8:N) ,'-k',...
          x1(1:8:N,:)',x2(1:8:N,:)',f(1:8:N,:)','-k');
    view(0,140);
    hcl=camlight('headlight');
    view(28,65);
    axis(ax);
    set(gca,'Fontsize',14);
    xlabel('X1','Fontsize',16);
    ylabel('X2','Fontsize',16);
    zlabel('f','Fontsize',16);
    title('rosenbrock','fontsize',16);
    imagename=[];
    if(nargin>1)
        imagename='rosen_3d_surf.png';
        print('-dpng',[path_to_save_images imagename]);
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_rosenbrock_contour(fig,ax,iflegend,...
    path_to_save_images)
    N=65;
    x1=repmat(((0:N-1)/(N-1))'*(ax(2)-ax(1))+ax(1),1,N);
    x2=repmat(((0:N-1)/(N-1)) *(ax(4)-ax(3))+ax(3),N,1);
    f=100*(x2-x1.^2).^2+(1-x1).^2;
    figure(fig);
    %V=0:50:ax(6);
    contour(x1,x2,f,80,'linestyle','-','linecolor',0.5*ones(1,3));
    axis equal;
    axis(ax(1:4));
    set(gca,'Fontsize',14);
    xlabel('X1','Fontsize',16);
    ylabel('X2','Fontsize',16);
    if(iflegend)
       set(legend('rosenbrock','location','southwest'),'fontsize'); 
    end
    imagename=[];
    if(nargin>3)
        imagename='rosen_2d_surf.png';
        print('-dpng',[path_to_save_images imagename]);
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_textbook_contour(fig,ax,ifcon,iflegend,...
    path_to_save_images,fname)
    N=65;
    x1=repmat(((0:N-1)/(N-1))'*(ax(2)-ax(1))+ax(1),1,N);
    x2=repmat(((0:N-1)/(N-1)) *(ax(4)-ax(3))+ax(3),N,1);
    f=(x1-1).^4+(x2-1).^4;
    [c,h]=contour(x1,x2,f,74,'linestyle','-','linecolor',0.5*ones(1,3));
    %clabel(c,h);
    axis equal;
    axis(ax(1:4));
    set(gca,'Fontsize',14);
    xlabel('X1','Fontsize',16);
    ylabel('X2','Fontsize',16); 
    if(ifcon)
        %plot constraints
        %g1(x1,x2) is (x1.^2-0.5*x2)<0; %contstaint 1 solid
        %g2(x1,x2) is (x2.^2-0.5*x1)<0; %constraint 2 dashed
        t=-32:.1:32;
        x1g1=t/4;
        x2g1=t.^2/8;
        x1g2=x2g1;
        x2g2=x1g1;
        hold on; 
        plot(x1g1,x2g1,'-',x1g2,x2g2,'--','linewidth',2,'color',[0 0.8 0]);
        if(iflegend)            
            h=legend('textbook','constraint g1<0','constraint g2<0',...
                'location','southwest');
            set(h,'fontsize',14);

            %h=legend('constraints','g1<0','g2<0','location','southwest');
            %hchild=get(h,'children');
            %delete(hchild(7));
        end
        hold off;
    end
    imagename=[];
    if(nargin>4)
        imagename=fname;
        print('-dpng',[path_to_save_images imagename]);        
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_rosenbrock_2d_param_study(...
    path_to_examples,ax,path_to_save_images)

    fid=fopen([path_to_examples ...
        'rosenbrock_2d_tabular.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'rosenbrock_2d_tabular.dat']);
    end
    fgetl(fid); %string of the header line
    data=fscanf(fid,'%g',[4 inf])';
    fclose(fid);
    fig=figure;
    plot_rosenbrock_contour(fig,ax,0); %(fig,ax,iflegend)
    hold on;
    plot(data(:,2),data(:,3),'o','markeredgecolor',[0.83 0 0],...
        'markerfacecolor',[0.83 0 0]);
    %set(legend('rosenbrock','param study','location','best'),...
    %    'fontsize',14);
    hold off;
    imagename=[];
    if(nargin>2)
        imagename='rosen_2d_pts.png';
        print('-dpng',[path_to_save_images imagename]);        
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_rosenbrock_vector_param_study(...
    path_to_examples,ax,path_to_save_images)

    fid=fopen([path_to_examples ...
        'rosenbrock_vector_tabular.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'rosenbrock_vector_tabular.dat']);
    end    
    fgetl(fid); %string of the header line
    data=fscanf(fid,'%g',[4 inf])';
    fclose(fid);
    fig=figure;
    plot_rosenbrock_contour(fig,ax,0); %(fig,ax,iflegend)
    hold on;
    plot(data(:,2),data(:,3),'o','markeredgecolor',[0.83 0 0],...
        'markerfacecolor',[0.83 0 0]);
    set(legend('rosenbrock','parameter study','location','southwest'),...
        'fontsize',14);
    hold off;
    imagename=[];
    if(nargin>2)
        imagename='rosen_vect_pts.png';
        print('-dpng',[path_to_save_images imagename]);        
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_rosenbrock_grad_opt(...
    path_to_examples,ax,path_to_save_images)

    fid=fopen([path_to_examples ...
        'rosenbrock_grad_opt_tabular.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'rosenbrock_grad_opt_tabular.dat']);
    end    
    fgetl(fid); %string of the header line
    data=fscanf(fid,'%g',[4 inf])';
    fclose(fid);
    fig=figure;
    plot_rosenbrock_contour(fig,ax,0); %(fig,ax,iflegend)
    hold on;
    plot(data(:,2),data(:,3),'o','markeredgecolor',[0.83 0 0],...
        'markerfacecolor',[0.83 0 0]);
    set(legend('rosenbrock','iteration history','location','southwest'),...
        'fontsize',14);
    hold off;
    imagename=[];
    if(nargin>2)
        imagename='rosen_grad_opt_pts.png';
        print('-dpng',[path_to_save_images imagename]);        
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_textbook_constrained_opt_example(...
    path_to_examples,ax,path_to_save_images)
    %this function generates the plot for the "constrained optimization of
    %the textbook function" example.  It requires that the
    %"dakota_tabular.dat" (in the example directory) output file for this 
    %example be moved to "textbook_tabular.dat" (in the example directory)
    %after running this function you will want to open the plot in gimp and
    %do an "image->autocrop image" save and also save as a .eps file
    fid=fopen([path_to_examples 'textbook_tabular.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'textbook_tabular.dat']);
    end    
    fgetl(fid); %string of the header line
    data=fscanf(fid,'%g',[6 inf])';
    fclose(fid);
    fig=figure;
    plot_textbook_contour(fig,ax,1,0); %(fig,ax,ifconstraints,iflegend)
    hold on;
    plot(data(:,2),data(:,3),'o','markeredgecolor',[0.83 0 0],...
        'markerfacecolor',[0.83 0 0]);
    set(legend('textbook','constraint g1<0','constraint g2<0',...
        'iteration history','location','southwest'),'fontsize',14);
    hold off; 
    imagename=[];
    if(nargin>2)
        imagename='textbook_history.png';
        print('-dpng',[path_to_save_images imagename]);
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_rosenbrock_ps_opt(path_to_examples,...
    ax,path_to_save_images,fname)

    fid=fopen([path_to_examples ...
        'rosenbrock_ps_opt_tabular.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'rosenbrock_ps_opt_tabular.dat']);
    end    
    fgetl(fid); %string of the header line
    data=fscanf(fid,'%g',[4 inf])';
    fclose(fid);
    fig=figure;
    plot_rosenbrock_contour(fig,ax,0); %(fig,ax,iflegend)
    hold on;
    %N=min(size(data,1),50);
    %h=text(data(1:N,2),data(1:N,3),num2str((1:N)')); %prints numbers
    %%instead of dots at data point locations
    %for ih=1:N
    %    set(h(ih),'HorizontalAlignment','center');
    %end
    plot(data(:,2),data(:,3),'o','markeredgecolor',[0.83 0 0],...
        'markerfacecolor',[0.83 0 0]);
    set(legend('rosenbrock','iteration history','location','southwest'),...
        'fontsize',14);
    hold off;
    imagename=[];
    if(nargin>2)
        imagename=fname;
        print('-dpng',[path_to_save_images imagename]);        
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_rosenbrock_ea_opt(path_to_examples,...
    ax,path_to_save_images,fname)

    fid=fopen([path_to_examples ...
        'rosenbrock_ea_opt_tabular.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'rosenbrock_ea_opt_tabular.dat']);
    end    
    fgetl(fid); %string of the header line
    data=fscanf(fid,'%g',[4 inf])';
    fclose(fid);
    warning(['plot_rosenbrock_ea_opt is assuming "population_size = 50"'...
        'and "replacement_type chc = 10"']);
    if(strcmp(fname,'rosen_ea_init.png'))
        data=data(1:50,:);
    elseif(strcmp(fname,'rosen_ea_final.png'))
        Npts=size(data,1);
        yada=sortrows(data(1:Npts-40,:),4);
        data=[yada(1:10,:); data(Npts-40+1:Npts,:)];
    else
       error('%s is an unknown filename',fname); 
    end
    fig=figure;
    plot_rosenbrock_contour(fig,ax,0); %(fig,ax,iflegend)
    hold on;
    plot(data(:,2),data(:,3),'o','markeredgecolor',[0.83 0 0],...
        'markerfacecolor',[0.83 0 0]);
    if(strcmp(fname,'rosen_ea_init.png'))
        %the location of this legend is hardcoded so as to not block any
        %data points given the current behavior of dakota (i.e. dakota 5.0)
        h=legend('rosenbrock','initial pop','location','southeast');
        set(h,'fontsize',14);
        pos=get(h,'position');
        pos(2)=pos(2)*1.1;
        set(h,'position',pos);
        %save DEVELOPME;
        %bob;
    elseif(strcmp(fname,'rosen_ea_final.png'))
        set(legend('rosenbrock','final pop','location','southwest'),...
            'fontsize',14);    
    end
    hold off;
    imagename=[];
    if(nargin>2)
        imagename=fname;
        print('-dpng',[path_to_save_images imagename]);        
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_mogatest1(path_to_examples,...
    path_to_save_images)
    %this function attempts to autoselect good axis bounds for the plot,
    %but parameters in the selection logic (i.e. the spacing of tick marks)
    %assume that the general range of the output does not change MUCH from
    %their current values (dakota release 5.0)

    fid=fopen([path_to_examples 'mogatest1_finaldata1.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'mogatest1_finaldata1.dat']);
    end    
    data=fscanf(fid,'%g',[5 inf])';
    fclose(fid);
    plot(data(:,4),data(:,5),'*','markeredgecolor',[0 0 0.87]);
    minmoga=floor(min(data(:,4:5)));
    maxmoga=ceil(max(data(:,4:5)));
    axis equal;
    axis(reshape([minmoga; maxmoga],1,4));
    set(gca,'Xtick',minmoga(1):0.1:maxmoga(1),'Ytick',...
        minmoga(2):0.1:maxmoga(2),'Fontsize',12)
    title('MOGA Test Problem #1 - Concave Pareto Frontier','Fontsize',14);
        xlabel('F1','Fontsize',14);
    ylabel('F2','Fontsize',14);
    imagename=[];
    if(nargin>1)
        imagename='dakota_mogatest1_pareto_front.png';
        print('-dpng',[path_to_save_images imagename]);
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_rosenbrock_nond(...
    path_to_examples,ax,path_to_save_images)

    fid=fopen([path_to_examples ...
        'rosenbrock_nond_tabular.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'rosenbrock_nond_tabular.dat']);
    end 
    fgetl(fid); %string of the header line
    data=fscanf(fid,'%g',[4 inf])';
    fclose(fid);
    fig=figure;
    plot_rosenbrock_contour(fig,ax,0); %(fig,ax,iflegend)
    hold on;
    plot(data(:,2),data(:,3),'o','markeredgecolor',[0.83 0 0],...
        'markerfacecolor',[0.83 0 0]);
    %set(legend('rosenbrock','nond','location','best'),...
    %    'fontsize',14);
    hold off;
    imagename=[];
    if(nargin>2)
        imagename='rosen_nond_pts.png';
        print('-dpng',[path_to_save_images imagename]);
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_rosenbrock_pce(...
    path_to_examples,ax,path_to_save_images)

    fid=fopen([path_to_examples ...
        'rosenbrock_pce_tabular.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'rosenbrock_pce_tabular.dat']);
    end 
    fgetl(fid); %string of the header line
    data=fscanf(fid,'%g',[4 inf])';
    fclose(fid);
    fig=figure;
    plot_rosenbrock_contour(fig,ax,0); %(fig,ax,iflegend)
    hold on;
    plot(data(:,2),data(:,3),'o','markeredgecolor',[0.83 0 0],...
        'markerfacecolor',[0.83 0 0]);
    h=legend('rosenbrock','TP Gauss. pts','location','southwest');
        set(h,'fontsize',14);
        pos=get(h,'position');
        pos(2)=pos(2)*2;
        set(h,'position',pos);
    %set(legend('rosenbrock','nond','location','best'),...
    %    'fontsize',14);
    hold off;
    imagename=[];
    if(nargin>2)
        imagename='rosen_pce_pts.png';
        print('-dpng',[path_to_save_images imagename]);
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


function imagename=plot_rosenbrock_sc(path_to_examples,ax,...
    path_to_save_images)

    fid=fopen([path_to_examples ...
        'nond_stoch_collocation_tabular.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'nond_stoch_collocation_tabular.dat']);
    end 
    fgetl(fid); %string of the header line
    data=fscanf(fid,'%g',[4 inf])';
    fclose(fid);

    fig=figure;
    %plot_rosenbrock_contour(fig,ax,0); 
    %hold on;
    plot(data(:,2),data(:,3),'o','markeredgecolor',[0.83 0 0],'markerfacecolor',[0.83 0 0]);
    grid on;
    %axis normal;
    axis(ax);
    %hold off; 
    set(gca,'Fontsize',14);
    xlabel('X1','Fontsize',16);
    ylabel('X2','Fontsize',16);
    set(legend('Collocation Pts','location','best'),'fontsize',14);
    %set(legend('rosenbrock','Collocation Pts','location','northeast'),'fontsize',14);

    imagename=[];
    if(nargin>2)
        imagename='rosen_sc_pts.png';
        print('-dpng',[path_to_save_images imagename]);
    end
end







%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_mogatest2(path_to_examples,...
    path_to_save_images)
    %this function attempts to autoselect good axis bounds for the plot,
    %but parameters in the selection logic (i.e. the spacing of tick marks)
    %assume that the general range of the output does not change MUCH from
    %their current values (dakota release 5.0)
    
    fid=fopen([path_to_examples ...
        'mogatest2_finaldata1.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'mogatest2_finaldata1.dat']);
    end 
    data=fscanf(fid,'%g',[4 inf])';
    fclose(fid);
    plot(data(:,3),data(:,4),'*','markeredgecolor',[0 0 0.87]);
    minmoga=floor(min(data(:,3:4)).*[10 5])./[10 5];
    maxmoga=ceil( max(data(:,3:4)).*[10 5])./[10 5];
    
    axis(reshape([minmoga; maxmoga],1,4));
    set(gca,'Xtick',minmoga(1):0.1:maxmoga(1),'Ytick',...
        minmoga(2):0.2:maxmoga(2),'Fontsize',12)
    title('MOGA Test Problem #2 - Discrete Pareto Frontier','Fontsize',14);
    xlabel('F1','Fontsize',14);
    ylabel('F2','Fontsize',14);
    
    imagename=[];
    if(nargin>1)
        imagename='dakota_mogatest2_pareto_front.png';
        print('-dpng',[path_to_save_images imagename]);
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function imagename=plot_mogatest3(path_to_examples,...
    path_to_save_images,fname)
    %this function attempts to autoselect good axis bounds for the plot,
    %but parameters in the selection logic (i.e. the spacing of tick marks)
    %assume that the general range of the output does not change MUCH from
    %their current values (dakota release 5.0)
    fid=fopen([path_to_examples ...
        'mogatest3_finaldata1.dat'],'r');
    if(fid==-1)
        error('Could not find file: "%s"',[path_to_examples ...
        'mogatest3_finaldata1.dat']);
    end 
    
    data=fscanf(fid,'%g',[6 inf])';
    fclose(fid);
    imagename=[];
    
    
    if((nargin==1)||strcmp(fname,'dakota_mogatest3_pareto_set.png'))
        figure;
        plot(data(:,1),data(:,2),'*','markeredgecolor',[0.83 0 0],'markerfacecolor',[0.83 0 0]);
        minmoga=floor(min(data(:,1:2))./[1 2]).*[1 2];
        maxmoga=ceil( max(data(:,1:2))./[1 2]).*[1 2];
        axis(reshape([minmoga; maxmoga],1,4));
        set(gca,'Fontsize',12)
        title('MOGA Test Problem #3 - Pareto Set','Fontsize',14);
        xlabel('X1','Fontsize',14);
        ylabel('X2','Fontsize',14);
    end
    
    if((nargin==1)||strcmp(fname,'dakota_mogatest3_pareto_front.png'))
        figure;
        plot(data(:,3),data(:,4),'*','markeredgecolor',[0 0 0.87]);
        minmoga=floor(min(data(:,3:4))/50)*50;
        maxmoga=ceil( max(data(:,3:4))/50)*50;
        axis equal;
        axis(reshape([minmoga; maxmoga],1,4));
        set(gca,'Fontsize',12)
        title('MOGA Test Problem #3 - Linear Pareto Frontier','Fontsize',14);
        xlabel('F1','Fontsize',14);
        ylabel('F2','Fontsize',14);
    end
    
    if(nargin==2)
        error(['a single call of this function will produce at most one'...
            ' image file (the images filename is passed in as the third'...
            ' argument of this subfunction)']);
    end
    
    if(nargin>2)
        imagename=[imagename fname]; %the
        %leading space is important because it serves to separate the 2
        %filenames produced by this function.
        print('-dpng',[path_to_save_images imagename]);
    end
end
