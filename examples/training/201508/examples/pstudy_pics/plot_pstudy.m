bounds = [0.4, 1.6, 0.8, 3.2];
msz = 2;
fsz = 10;

figure(1);

cent_pts = load('centered_pstudy.dat');
plot(cent_pts(:,2), cent_pts(:,3), ...
     'bo', 'MarkerSize', msz, 'LineWidth',2.0);
xlabel('x_1', 'FontSize', fsz, 'FontWeight', 'bold');
ylabel('x_2', 'FontSize', fsz, 'FontWeight', 'bold');
title('Centered Parameter Study', 'FontSize', fsz, 'FontWeight', 'bold');
set(gca, 'FontSize',fsz);
set(gca, 'FontWeight', 'bold');
axis(bounds);
grid on
hold on
rectangle('Position', [0.5, 1.0, 1.0, 2.0])
hold off

set(1, 'PaperSize', [4,3]);
set(1, 'PaperPosition', [0,0,4,3])
print -dpng demo_centered.png
%print -dpdf demo_centered.png


figure(2);

grid_pts = load('grid_pstudy.dat');
plot(grid_pts(:,2), grid_pts(:,3), ...
     'bo', 'MarkerSize', msz, 'LineWidth',2.0);
xlabel('x_1', 'FontSize', fsz, 'FontWeight', 'bold');
ylabel('x_2', 'FontSize', fsz, 'FontWeight', 'bold');
title('Multidim Parameter Study', 'FontSize', fsz, 'FontWeight', 'bold');
set(gca, 'FontSize',fsz);
set(gca, 'FontWeight', 'bold');
axis(bounds);
grid on
hold on
rectangle('Position', [0.5, 1.0, 1.0, 2.0])
hold off

set(2, 'PaperSize', [4,3]);
set(2, 'PaperPosition', [0,0,4,3])
print -dpng demo_grid.png
%print -dpdf demo_grid.png


figure(3);

unif_pts = load('lhs_uniform.dat');
plot(unif_pts(:,2), unif_pts(:,3), ...
     'bo', 'MarkerSize', msz, 'LineWidth',2.0);
xlabel('x_1', 'FontSize', fsz, 'FontWeight', 'bold');
ylabel('x_2', 'FontSize', fsz, 'FontWeight', 'bold');
title('Uniform LHS', 'FontSize', fsz, 'FontWeight', 'bold');
set(gca, 'FontSize',fsz);
set(gca, 'FontWeight', 'bold');
axis(bounds);
set(gca,'XTick',0.4:.1:1.6)
set(gca,'YTick',0.8:.2:3.2)
grid on
hold on
rectangle('Position', [0.5, 1.0, 1.0, 2.0])
hold off

set(3, 'PaperSize', [4,3]);
set(3, 'PaperPosition', [0,0,4,3])
print -dpng demo_lhs_unif.png
%print -dpdf demo_lhs_unif.png


figure(4);

norm_pts = load('lhs_normal.dat');
plot(norm_pts(:,2), norm_pts(:,3), ...
     'bo', 'MarkerSize', msz, 'LineWidth',2.0);
xlabel('x_1', 'FontSize', fsz, 'FontWeight', 'bold');
ylabel('x_2', 'FontSize', fsz, 'FontWeight', 'bold');
title('Normal LHS', 'FontSize', fsz, 'FontWeight', 'bold');
set(gca, 'FontSize',fsz);
set(gca, 'FontWeight', 'bold');

axis([0.5, 1.5, 1.0, 3.0]);

% add lines to show equi-probable bins
hold on

equiprob_x = norminv(0.1:0.1:0.9, 1.0, 0.10);
bounds_y = repmat([1.0;3.0],1,9);
line([equiprob_x;equiprob_x], bounds_y, 'LineStyle', ':', 'Color', 'k');

equiprob_y = norminv(0.1:0.1:0.9, 2.0, 0.50);
bounds_x = repmat([0.5;1.5],1,9);
line(bounds_x, [equiprob_y;equiprob_y], 'LineStyle', ':', 'Color', 'k');

xvals = 0.5:0.05:1.5;
pdfvals = normpdf(xvals, 1.0, 0.1);
plot(xvals, 1.0+0.2*pdfvals/max(pdfvals));

hold off

set(4, 'PaperSize', [4,3]);
set(4, 'PaperPosition', [0,0,4,3])
print -dpng demo_lhs_norm.png
%print -dpdf demo_lhs_norm.png
