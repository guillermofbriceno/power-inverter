clc
clear
pkg load signal

sine_freq = 60;
ramp_freq = 60000;
mcu_freq = 80000000;
sample_period = 0.0000001;
atten_sine = 0.98;
num_periods = 1;

pwm_period_in_cycles = floor(mcu_freq / ramp_freq);
t = 0:sample_period:(1/sine_freq) * num_periods;
max_time = ceil((1/sine_freq * num_periods) / sample_period);

pwm = [];

saw = sawtooth(2*pi*ramp_freq*t);
sine = atten_sine*sin(2*pi*sine_freq*t);

for i = 1:max_time
  if sine(i) > saw(i)
    pwm(i) = 1;
  elseif sine(i) < saw(i)
    pwm(i) = 0;
  endif
endfor

pwm_dutycycles = [];
count_high = 0;
count_low = 0;
elapsed_period = 0;
count = 0;

for i = 1:max_time
  if elapsed_period >= 1/ramp_freq
    elapsed_period = 0;
    count++;
    pwm_dutycycles(count) = round((count_high / (count_high + count_low)) * pwm_period_in_cycles);
    count_high = 0;
    count_low = 0;
  elseif pwm(i) == 1
    count_high++;
  elseif pwm(i) == 0
    count_low++;
  endif
  
  elapsed_period = t(i) - (1/ramp_freq) * count;
endfor

fileID = fopen('/home/guillermo/school/fall-2019/design/pwm.out', 'w');
fprintf(fileID,'const uint32_t pwmCycles[] = {');

for i = 1:length(pwm_dutycycles)
  if i < length(pwm_dutycycles)
    fprintf(fileID,'%d,',pwm_dutycycles(i));
  else
    fprintf(fileID,'%d',pwm_dutycycles(i));
  endif
endfor

fprintf(fileID,'};');
fclose(fileID);

%plot(t, saw, "linewidth", 1);
%hold on;
%plot(t, sine, "linewidth", 5);
%hold on;
%plot(t, pwm, "linewidth", 2);
