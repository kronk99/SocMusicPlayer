% filter_sweep.m
% Script para aplicar 5 filtros al barrido generado
% Proyecto: SocMusicPlayer - Sistemas Empotrados

pkg load signal
clear all;
close all;
clc;

% ======================================
% Cargar el sweep generado
% ======================================
fprintf('Cargando barrido de frecuencias...\n');

filename = 'sweep_generado.wav';
if ~exist(filename, 'file')
    error('Error: No se encontró "%s".\nPrimero ejecuta "generate_sweep.m" para crear el barrido.', filename);
end

[sweep, Fs] = audioread(filename);

% Convertir a mono si es estéreo
if size(sweep, 2) > 1
    sweep = mean(sweep, 2);
end

N = length(sweep);
duration = N / Fs;

fprintf('Barrido cargado exitosamente!\n');
fprintf('  Frecuencia de muestreo: %d Hz\n', Fs);
fprintf('  Duración: %.2f segundos\n', duration);
fprintf('  Número de muestras: %d\n\n', N);

% ======================================
% Diseño de filtros optimizados para el sweep
% ======================================
fprintf('========================================\n');
fprintf('   FILTROS PARA BARRIDO 20Hz-20kHz\n');
fprintf('========================================\n\n');

% Filtro 1: Pasa-Bajos (elimina frecuencias > 2000 Hz)
fprintf('1. Filtro Pasa-Bajos:\n');
fprintf('   - Deja pasar: 20 Hz a 2000 Hz\n');
fprintf('   - Elimina: > 2000 Hz\n');
fc_low = 2000;
orden_pb = 10;
[b_pb, a_pb] = butter(orden_pb, fc_low/(Fs/2), 'low');
sweep_pb = filter(b_pb, a_pb, sweep);
fprintf('   ✓ Filtro diseñado (orden %d)\n\n', orden_pb);

% Filtro 2: Pasa-Altos (elimina frecuencias < 2000 Hz)
fprintf('2. Filtro Pasa-Altos:\n');
fprintf('   - Elimina: < 2000 Hz\n');
fprintf('   - Deja pasar: 2000 Hz a 20000 Hz\n');
fc_high = 2000;
orden_pa = 10;
[b_pa, a_pa] = butter(orden_pa, fc_high/(Fs/2), 'high');
sweep_pa = filter(b_pa, a_pa, sweep);
fprintf('   ✓ Filtro diseñado (orden %d)\n\n', orden_pa);

% Filtro 3: Pasa-Banda (solo 1000-4000 Hz)
fprintf('3. Filtro Pasa-Banda:\n');
fprintf('   - Elimina: < 1000 Hz y > 4000 Hz\n');
fprintf('   - Deja pasar: 1000 Hz a 4000 Hz\n');
f_band_low = 1000;
f_band_high = 4000;
orden_band = 8;
[b_band, a_band] = butter(orden_band, [f_band_low, f_band_high]/(Fs/2), 'bandpass');
sweep_band = filter(b_band, a_band, sweep);
fprintf('   ✓ Filtro diseñado (orden %d)\n\n', orden_band);

% Filtro 4: Rechaza-Banda (elimina 1000-4000 Hz)
fprintf('4. Filtro Rechaza-Banda (Notch):\n');
fprintf('   - Deja pasar: < 1000 Hz y > 4000 Hz\n');
fprintf('   - Elimina: 1000 Hz a 4000 Hz\n');
f_notch_low = 1000;
f_notch_high = 4000;
orden_notch = 8;
[b_notch, a_notch] = butter(orden_notch, [f_notch_low, f_notch_high]/(Fs/2), 'stop');
sweep_notch = filter(b_notch, a_notch, sweep);
fprintf('   ✓ Filtro diseñado (orden %d)\n\n', orden_notch);

% Filtro 5: Ecualización (realza 1000-4000 Hz)
fprintf('5. Filtro de Ecualización:\n');
fprintf('   - Realza: 1000 Hz a 4000 Hz (4x)\n');
fprintf('   - Mantiene resto del espectro\n');
f_eq_low = 1000;
f_eq_high = 4000;
orden_eq = 8;
ganancia = 4.0;
[b_eq, a_eq] = butter(orden_eq, [f_eq_low, f_eq_high]/(Fs/2), 'bandpass');
medios = filter(b_eq, a_eq, sweep);
sweep_eq = sweep + ganancia * medios;
sweep_eq = sweep_eq / max(abs(sweep_eq));  % Normalizar
fprintf('   ✓ Filtro diseñado (orden %d, ganancia %.1fx)\n\n', orden_eq, ganancia);

fprintf('========================================\n\n');

% ======================================
% Menú interactivo
% ======================================
while true
    fprintf('========================================\n');
    fprintf('   MENÚ DE REPRODUCCIÓN Y ANÁLISIS\n');
    fprintf('========================================\n');
    fprintf('1. Reproducir barrido ORIGINAL (20Hz-20kHz)\n');
    fprintf('2. Reproducir PASA-BAJOS (solo 20Hz-2kHz)\n');
    fprintf('3. Reproducir PASA-ALTOS (solo 2kHz-20kHz)\n');
    fprintf('4. Reproducir PASA-BANDA (solo 1kHz-4kHz)\n');
    fprintf('5. Reproducir RECHAZA-BANDA (hueco 1kHz-4kHz)\n');
    fprintf('6. Reproducir ECUALIZACIÓN (realce 1kHz-4kHz)\n');
    fprintf('7. Ver análisis espectral comparativo\n');
    fprintf('8. Ver espectrogramas comparativos\n');
    fprintf('9. Guardar todos los audios filtrados\n');
    fprintf('0. Salir\n');
    fprintf('========================================\n');

    opcion = input('Selecciona una opción: ');

    if opcion == 0
        fprintf('\nSaliendo del programa...\n');
        break;
    end

    switch opcion
        case 1
            fprintf('\n--- BARRIDO ORIGINAL ---\n');
            fprintf('Reproduciendo sweep completo (20 Hz a 20 kHz)...\n');
            fprintf('Escucharás un tono que sube continuamente de grave a agudo.\n\n');
            player = audioplayer(sweep, Fs);
            play(player);

        case 2
            fprintf('\n--- FILTRO PASA-BAJOS ---\n');
            fprintf('Reproduciendo solo frecuencias BAJAS (20 Hz a 2 kHz)...\n');
            fprintf('El barrido se DETENDRÁ en 2 kHz (tonos medios-graves).\n\n');
            player = audioplayer(sweep_pb, Fs);
            play(player);

        case 3
            fprintf('\n--- FILTRO PASA-ALTOS ---\n');
            fprintf('Reproduciendo solo frecuencias ALTAS (2 kHz a 20 kHz)...\n');
            fprintf('El barrido COMENZARÁ en 2 kHz (no oirás tonos graves).\n\n');
            player = audioplayer(sweep_pa, Fs);
            play(player);

        case 4
            fprintf('\n--- FILTRO PASA-BANDA ---\n');
            fprintf('Reproduciendo solo frecuencias MEDIAS (1 kHz a 4 kHz)...\n');
            fprintf('Solo oirás una porción CENTRAL del barrido.\n\n');
            player = audioplayer(sweep_band, Fs);
            play(player);

        case 5
            fprintf('\n--- FILTRO RECHAZA-BANDA ---\n');
            fprintf('Reproduciendo con HUECO en frecuencias medias (1 kHz a 4 kHz)...\n');
            fprintf('Oirás el inicio y el final, pero habrá un SILENCIO en medio.\n\n');
            player = audioplayer(sweep_notch, Fs);
            play(player);

        case 6
            fprintf('\n--- FILTRO DE ECUALIZACIÓN ---\n');
            fprintf('Reproduciendo con REALCE en frecuencias medias (1 kHz a 4 kHz)...\n');
            fprintf('La parte central del barrido sonará MÁS FUERTE.\n\n');
            player = audioplayer(sweep_eq, Fs);
            play(player);

        case 7
            fprintf('\n--- ANÁLISIS ESPECTRAL ---\n');
            fprintf('Generando gráficas de espectros de frecuencia...\n\n');

            % Calcular FFTs
            NFFT = 2^nextpow2(N);
            f = Fs/2 * linspace(0, 1, NFFT/2+1);

            Y_orig = fft(sweep, NFFT)/N;
            Y_pb = fft(sweep_pb, NFFT)/N;
            Y_pa = fft(sweep_pa, NFFT)/N;
            Y_band = fft(sweep_band, NFFT)/N;
            Y_notch = fft(sweep_notch, NFFT)/N;
            Y_eq = fft(sweep_eq, NFFT)/N;

            % Crear figura
            figure('Position', [50, 50, 1400, 900]);

            % Original
            subplot(3, 2, 1);
            plot(f, 2*abs(Y_orig(1:NFFT/2+1)));
            title('Espectro - Barrido Original (20Hz-20kHz)');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 10000]);
            line([2000, 2000], ylim, 'Color', 'r', 'LineStyle', '--', 'LineWidth', 1);
            legend('Espectro', 'fc=2kHz', 'Location', 'best');

            % Pasa-bajos
            subplot(3, 2, 2);
            plot(f, 2*abs(Y_pb(1:NFFT/2+1)));
            title('Pasa-Bajos: Solo < 2kHz (orden=10)');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 10000]);
            line([2000, 2000], ylim, 'Color', 'r', 'LineStyle', '--', 'LineWidth', 2);
            legend('Filtrado', 'fc=2kHz', 'Location', 'best');

            % Pasa-altos
            subplot(3, 2, 3);
            plot(f, 2*abs(Y_pa(1:NFFT/2+1)));
            title('Pasa-Altos: Solo > 2kHz (orden=10)');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 10000]);
            line([2000, 2000], ylim, 'Color', 'r', 'LineStyle', '--', 'LineWidth', 2);
            legend('Filtrado', 'fc=2kHz', 'Location', 'best');

            % Pasa-banda
            subplot(3, 2, 4);
            plot(f, 2*abs(Y_band(1:NFFT/2+1)));
            title('Pasa-Banda: Solo 1kHz-4kHz (orden=8)');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 10000]);
            line([1000, 1000], ylim, 'Color', 'r', 'LineStyle', '--', 'LineWidth', 1);
            line([4000, 4000], ylim, 'Color', 'r', 'LineStyle', '--', 'LineWidth', 1);
            legend('Filtrado', 'Cortes', 'Location', 'best');

            % Rechaza-banda
            subplot(3, 2, 5);
            plot(f, 2*abs(Y_notch(1:NFFT/2+1)));
            title('Rechaza-Banda: Hueco 1kHz-4kHz (orden=8)');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 10000]);
            line([1000, 1000], ylim, 'Color', 'r', 'LineStyle', '--', 'LineWidth', 1);
            line([4000, 4000], ylim, 'Color', 'r', 'LineStyle', '--', 'LineWidth', 1);
            legend('Filtrado', 'Cortes', 'Location', 'best');

            % Ecualización
            subplot(3, 2, 6);
            plot(f, 2*abs(Y_eq(1:NFFT/2+1)));
            title('Ecualización: Realce 1kHz-4kHz (4x)');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 10000]);
            line([1000, 1000], ylim, 'Color', 'g', 'LineStyle', '--', 'LineWidth', 1);
            line([4000, 4000], ylim, 'Color', 'g', 'LineStyle', '--', 'LineWidth', 1);
            legend('Ecualizado', 'Banda realzada', 'Location', 'best');

            fprintf('¡Gráficas generadas!\n');

        case 8
            fprintf('\n--- ESPECTROGRAMAS COMPARATIVOS ---\n');
            fprintf('Generando espectrogramas (puede tardar un momento)...\n\n');

            figure('Position', [50, 50, 1400, 1000]);

            % Original
            subplot(3, 2, 1);
            spectrogram(sweep, 2048, 1024, 2048, Fs, 'yaxis');
            title('Espectrograma - Original');
            colorbar;
            ylim([0, 20]);

            % Pasa-bajos
            subplot(3, 2, 2);
            spectrogram(sweep_pb, 2048, 1024, 2048, Fs, 'yaxis');
            title('Espectrograma - Pasa-Bajos (< 2kHz)');
            colorbar;
            ylim([0, 20]);

            % Pasa-altos
            subplot(3, 2, 3);
            spectrogram(sweep_pa, 2048, 1024, 2048, Fs, 'yaxis');
            title('Espectrograma - Pasa-Altos (> 2kHz)');
            colorbar;
            ylim([0, 20]);

            % Pasa-banda
            subplot(3, 2, 4);
            spectrogram(sweep_band, 2048, 1024, 2048, Fs, 'yaxis');
            title('Espectrograma - Pasa-Banda (1-4kHz)');
            colorbar;
            ylim([0, 20]);

            % Rechaza-banda
            subplot(3, 2, 5);
            spectrogram(sweep_notch, 2048, 1024, 2048, Fs, 'yaxis');
            title('Espectrograma - Rechaza-Banda');
            colorbar;
            ylim([0, 20]);

            % Ecualización
            subplot(3, 2, 6);
            spectrogram(sweep_eq, 2048, 1024, 2048, Fs, 'yaxis');
            title('Espectrograma - Ecualización');
            colorbar;
            ylim([0, 20]);

            fprintf('¡Espectrogramas generados!\n');

        case 9
            fprintf('\n--- GUARDAR AUDIOS FILTRADOS ---\n');
            fprintf('Guardando archivos...\n\n');

            audiowrite('sweep_pasa_bajos.wav', sweep_pb, Fs);
            fprintf('  ✓ sweep_pasa_bajos.wav\n');

            audiowrite('sweep_pasa_altos.wav', sweep_pa, Fs);
            fprintf('  ✓ sweep_pasa_altos.wav\n');

            audiowrite('sweep_pasa_banda.wav', sweep_band, Fs);
            fprintf('  ✓ sweep_pasa_banda.wav\n');

            audiowrite('sweep_rechaza_banda.wav', sweep_notch, Fs);
            fprintf('  ✓ sweep_rechaza_banda.wav\n');

            audiowrite('sweep_ecualizacion.wav', sweep_eq, Fs);
            fprintf('  ✓ sweep_ecualizacion.wav\n');

            fprintf('\n¡Todos los archivos guardados exitosamente!\n');

        otherwise
            fprintf('\nOpción inválida. Por favor, selecciona una opción del 0 al 9.\n');
    end

    fprintf('\n');
    input('Presiona Enter para continuar...');
    clc;
end
