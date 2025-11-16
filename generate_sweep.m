% generate_sweep.m
% Script para generar un barrido de frecuencias (sweep)
% Proyecto: SocMusicPlayer - Sistemas Empotrados

pkg load signal
clear all;
close all;
clc;

fprintf('========================================\n');
fprintf('  GENERADOR DE BARRIDO DE FRECUENCIAS\n');
fprintf('========================================\n\n');

% ======================================
% Parámetros del sweep
% ======================================
Fs = 44100;           % Frecuencia de muestreo (Hz)
duration = 10;        % Duración del sweep (segundos)
f_start = 20;         % Frecuencia inicial (Hz)
f_end = 20000;        % Frecuencia final (Hz)
amplitude = 0.8;      % Amplitud (0-1)

fprintf('Parámetros del sweep:\n');
fprintf('  Frecuencia de muestreo: %d Hz\n', Fs);
fprintf('  Duración: %d segundos\n', duration);
fprintf('  Frecuencia inicial: %d Hz\n', f_start);
fprintf('  Frecuencia final: %d Hz\n', f_end);
fprintf('  Tipo: Barrido logarítmico\n\n');

% ======================================
% Generar el sweep
% ======================================
fprintf('Generando barrido de frecuencias...\n');

% Vector de tiempo
t = linspace(0, duration, Fs * duration);

% Generar sweep logarítmico
% La frecuencia instantánea varía logarítmicamente con el tiempo
sweep = chirp(t, f_start, duration, f_end, 'logarithmic');

% Aplicar amplitud
sweep = amplitude * sweep;

fprintf('¡Barrido generado exitosamente!\n');
fprintf('  Número de muestras: %d\n\n', length(sweep));

% ======================================
% Guardar el archivo
% ======================================
filename = 'sweep_generado.wav';
audiowrite(filename, sweep, Fs);
fprintf('Archivo guardado como: %s\n\n', filename);

% ======================================
% Análisis espectral
% ======================================
fprintf('Generando análisis espectral...\n');

% Espectrograma
figure('Position', [100, 100, 1200, 800]);

subplot(3, 1, 1);
plot(t, sweep);
title('Forma de Onda del Sweep');
xlabel('Tiempo (s)');
ylabel('Amplitud');
grid on;
xlim([0, duration]);

subplot(3, 1, 2);
spectrogram(sweep, 2048, 1024, 2048, Fs, 'yaxis');
title('Espectrograma - Barrido Logarítmico (20 Hz a 20 kHz)');
colorbar;
ylim([0, 20]);

% FFT
subplot(3, 1, 3);
NFFT = 2^nextpow2(length(sweep));
Y = fft(sweep, NFFT);
f = Fs/2 * linspace(0, 1, NFFT/2+1);
magnitude = 2*abs(Y(1:NFFT/2+1)) / length(sweep);

plot(f, magnitude);
title('Espectro de Frecuencias');
xlabel('Frecuencia (Hz)');
ylabel('Magnitud');
grid on;
xlim([0, 20000]);
set(gca, 'XScale', 'log');

fprintf('¡Gráficas generadas!\n\n');

% ======================================
% Reproducir el sweep
% ======================================
fprintf('¿Deseas reproducir el sweep generado? (s/n): ');
respuesta = input('', 's');

if strcmpi(respuesta, 's')
    fprintf('\nReproduciendo barrido de frecuencias...\n');
    fprintf('(El tono subirá progresivamente de grave a agudo)\n');
    player = audioplayer(sweep, Fs);
    play(player);

    fprintf('\nPresiona Ctrl+C para detener.\n');
    pause(duration + 1);
end

fprintf('\n========================================\n');
fprintf('Proceso completado.\n');
fprintf('Ahora puedes usar "filter_sweep.m" para\n');
fprintf('aplicar filtros a este barrido.\n');
fprintf('========================================\n');
