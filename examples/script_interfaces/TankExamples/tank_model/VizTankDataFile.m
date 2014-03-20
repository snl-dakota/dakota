function [x_vec_reflect, phi_vec, Displ ] = VizTankDataFile(dataFile)
% VizTankDataFile.m
% khu 131229
% This parses the data file and plots a response in half symmetry (side
% view of the Tank).
% There are no input arguments, but you must specify the name of the
% dataFile, and select the response you wish to plot
% Sample outputs can be found in the V&V Challenge Problem Statment
% which is found online at https://share.sandia.gov/vvcw
% 
% 
% notes
% this really only works for resultStyle 1, or a full grid of X and Phi


%% select which responses to viz
Header = 'Normal_Displacement';
% Header = 'EFFECTIVE_STRESSES_Outboard';
% Header = 'EFFECTIVE_STRESSES_Inboard'; 

% Currently, only three responses above are written out. Others can be
% activated by uncommenting lines in the function reportResults in
% FEMTank.py

% Header = 'Axial_Displacement';
% Header = 'Circumferential_Displacement';
% Header = 'eps_x_Outboard';
% Header = 'eps_phi_Outboard';
% Header = 'gamma_x_phi_Outboard';
% Header = 'eps_x_Inboard';
% Header = 'eps_phi_Inboard';
% Header = 'gamma_x_phi_Inboard';
% Header = 'sig_x_Outboard';
% Header = 'sig_phi_Outboard';
% Header = 'sig_x_phi_Outboard';
% Header = 'sig_x_Inboard';
% Header = 'sig_phi_Inboard';
% Header = 'sig_x_phi_Inboard';

ResponseIsDisp = strfind(Header, 'Displacement');
Displ = [];

%% Parse the dataFile

fid = fopen(dataFile,'r');

test = textscan(fid, '%s %s %f', 'HeaderLines',1);
% will read until failure, so third cell contains all input values

for idx = 1:length(test{3})
    eval([ test{1}{idx}, ' = ', num2str(test{3}(idx)), ';']);
end

fgetl(fid); %burn next line 
x_vec = str2num(fgetl(fid));

x_vec_reflect = [ -fliplr(x_vec) x_vec ]; %show half the tank, instead of a quarter. it just looks nicer

fgetl(fid); %burn next line 
phi_vec = str2num(fgetl(fid));

% loop through blocks with format
% Header Line
% nX lines of nPhi columns, comma delimited
Headers = {};
line = fgetl(fid);
while line ~= -1
    %deal with fancy characters and spaces
    temp = strsplit(line, ',');
    temp = strrep(temp{1}, ' ','_');
    temp = strrep(temp, '(','');
    temp = strrep(temp, ')','');
    Headers{end+1} = temp;
    for idx = 1:length(x_vec)
        Values{length(Headers)}(idx,:) = str2num(fgetl(fid));
    end
    line = fgetl(fid);
end

fclose(fid);


%% Plot the response selected above
% See below for fancier version fo Stresses
for idx = 1:length(Headers)
    if strcmp(Headers{idx},Header)
        Response = [ fliplr(Values{idx}') Values{idx}'];
        break
    end
end
% find max absolute value
Cmax = max(abs(Response(:)));

figure
surf(x_vec_reflect,phi_vec,Response, 'linestyle', 'none')


ind = find(~(Cmax-abs(Response(:))));
[phiiind,xind] = ind2sub(size(Response),ind);
hold on
plot3(x_vec_reflect(xind), phi_vec(phiiind), Response(ind)+1, 'ok', 'markersize', 10, 'markerfacecolor','k')

title(strrep(Header, '_', '\_'))
view(2)
axis([-max(x_vec),max(x_vec),0,180])
colorbar
xlabel('Axial location (in)')
ylabel('Circumferencial Angel (^o)')


% handle colors
% displacements can be positive/negative, others are non-negative
if ResponseIsDisp
    caxis([-Cmax,Cmax])
    colormap(bipolar(201))
    Displ = Response;
else
    SemiHot = hot(round(201*1.3)); SemiHot = SemiHot(1:201,:); %don't get white hot. too hard to see things.
    caxis([0,Cmax])
    colormap(SemiHot)
end


%% Here's some customized code to plot the two stresses on a common color scale
% run this instead of the "Plot the response..." section above
StressHeaders = {'EFFECTIVE_STRESSES_Outboard', 'EFFECTIVE_STRESSES_Inboard'};
Cmax = 0;
for SHidx = 1:length(StressHeaders)
for idx = 1:length(Headers)
    if strcmp(Headers{idx},StressHeaders{SHidx})
        Responses{SHidx} = [ fliplr(Values{idx}') Values{idx}'];
        Cmax = max( Cmax, max(abs(Responses{SHidx}(:))) );
    end
end
end

SemiHot = hot(round(201*1.3)); SemiHot = SemiHot(1:201,:); %don't get white hot. too hard to see things.


for SHidx = 1:length(StressHeaders)
    Response = Responses{SHidx};

    figure
    surf(x_vec_reflect,phi_vec,Response, 'linestyle', 'none')

    ind = find(~(Cmax-abs(Response(:))));
    [phiiind,xind] = ind2sub(size(Response),ind);
    hold on
    plot3(x_vec_reflect(xind), phi_vec(phiiind), Response(ind)+1, 'ok', 'markersize', 10, 'markerfacecolor','k')

    title(strrep(StressHeaders{SHidx}, '_', '\_'))
    view(2)
    axis([-max(x_vec),max(x_vec),0,180])
    caxis([0,Cmax])
    colormap(SemiHot)
    colorbar
    xlabel('Axial location (in)')
    ylabel('Circumferencial Angel (^o)')
end
end