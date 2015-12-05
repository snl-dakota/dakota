dnominal = [ 2.2786458300000001e+02 2.6133333299999998e+04 1.8042803500000000e+00 ];

num_data = [1 5 10 50];
rel_std_dev = 0.10;
nl_str = '10';

rng(201508);

for exp_ind = 1:length(num_data),

  num_exp = num_data(exp_ind);
  data_mult = 1.0 + rel_std_dev*randn(num_exp, 3);

  noisy_data = data_mult .* repmat(dnominal, num_exp, 1);

save(['cant_exp_data.', nl_str, 'perc.', int2str(num_exp), '.dat'], 'noisy_data', '-ascii');

  %figure(1)
  %for i=1:3,
  %  subplot(1,3,i);
  %  plot(1,dnominal(:,i),'o',1,noisy_data(:,i),'x');
  %end

end
