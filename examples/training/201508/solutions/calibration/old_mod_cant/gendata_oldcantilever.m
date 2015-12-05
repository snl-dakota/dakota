dnominal = load('dakota_cantilever_examples.clean.dat');

rng(201508);

% generate data with various noise levels
num_exp = 1;
rel_std_dev = [0.5, 0.1, 0.05, 0.001];

for noise_ind = 1:4,
  
  data_mult = 1.0 + rel_std_dev(noise_ind)*randn(num_exp, 3);

  noisy_data = data_mult .* repmat(dnominal, num_exp, 1);

  save(['cant_exp_data.', int2str(noise_ind), '.dat'], 'noisy_data', '-ascii');

end



break

num_data = [1 5 10 50];
rel_std_dev = 0.001;


for exp_ind = 1:length(num_data),

  num_exp = num_data(exp_ind);
  data_mult = 1.0 + rel_std_dev*randn(num_exp, 3);

  noisy_data = data_mult .* repmat(dnominal, num_exp, 1);

  save(['old_cant_exp_data.', int2str(num_exp), '.dat'], 'noisy_data', '-ascii');

  %figure(1)
  %for i=1:3,
  %  subplot(1,3,i);
  %  plot(1,dnominal(:,i),'o',1,noisy_data(:,i),'x');
  %end

end


