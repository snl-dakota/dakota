lines(0);
warning_old = warning('query');

femtruss_path = get_absolute_file_path('init_bridge_optim_derivative.sce');
exec(femtruss_path + 'build_long_bridge_2d.sce');


