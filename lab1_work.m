n=500; 
x=randi(16,1,n)-1; % 500 random characters 0-15

result = bitshift(x(1:2:end),4) + x(2:2:end);

% modulation
y = qammod(result,256); % signal generation for QAM16 modulation

SNR_range = 2:30;
error_results = zeros(1, length(SNR_range));

for i = 1:length(SNR_range)

    % add white Gaussian Noise SNR 15 dB
    y_noise = awgn(y,SNR_range(i),'measured');
    %signal demodulation
    z = qamdemod(y_noise,256);

    z_unpacked = zeros(1,n);
    z_unpacked(1:2:end) = bitshift(z, -4);
    z_unpacked(2:2:end) = bitand(z,15);

    % error calculation
    error = z_unpacked - x; % non zero is error
    error_percentage = (nnz(error)/n) * 100;

    error_results(i) = error_percentage;
end

disp(error_results);

plot(SNR_range, error_results);
grid();
xlim([0,30]);
ylim([0,100]);
xlabel("SNR [dB]");
ylabel("STOPA BLEDU [%]");