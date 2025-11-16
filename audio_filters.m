% audio_filters.m
% Script para aplicar 5 filtros distintos al audio MLKDream.wav
% Proyecto: SocMusicPlayer - Sistemas Empotrados

pkg load signal
clear all;
close all;
clc;

% ======================================
% Cargar el archivo de audio
% ======================================
fprintf('Cargando archivo de audio...\n');
[audio, Fs] = audioread('long_sweep.wav');

% Convertir a mono si es estéreo
if size(audio, 2) > 1
    audio = mean(audio, 2);
end

N = length(audio);
duration = N / Fs;

fprintf('Archivo cargado exitosamente!\n');
fprintf('  Frecuencia de muestreo: %d Hz\n', Fs);
fprintf('  Duración: %.2f segundos\n', duration);
fprintf('  Número de muestras: %d\n\n', N);

% ======================================
% Menú principal
% ======================================
while true
    fprintf('========================================\n');
    fprintf('   FILTROS DE AUDIO - SocMusicPlayer\n');
    fprintf('========================================\n');
    fprintf('1. Filtro Pasa-Bajos (Low-Pass)\n');
    fprintf('2. Filtro Pasa-Altos (High-Pass)\n');
    fprintf('3. Filtro Pasa-Banda (Band-Pass)\n');
    fprintf('4. Filtro Rechaza-Banda (Band-Stop/Notch)\n');
    fprintf('5. Filtro de Ecualización (Realce de Medios)\n');
    fprintf('6. Reproducir audio original\n');
    fprintf('7. Ver análisis espectral comparativo\n');
    fprintf('0. Salir\n');
    fprintf('========================================\n');

    opcion = input('Selecciona una opción: ');

    if opcion == 0
        fprintf('\nSaliendo del programa...\n');
        break;
    end

    switch opcion
        case 1
            % ======================================
            % FILTRO PASA-BAJOS
            % ======================================
            fprintf('\n--- Filtro Pasa-Bajos ---\n');
            fprintf('Elimina frecuencias superiores a la frecuencia de corte.\n');
            fprintf('En un sweep, se oirá solo la parte de frecuencias bajas.\n\n');

            % Parámetros del filtro - MÁS AGRESIVO
            fc = 2000;  % Frecuencia de corte en Hz (más baja)
            orden = 10;  % Orden más alto = corte más pronunciado

            % Diseño del filtro Butterworth
            Wn = fc / (Fs/2);  % Frecuencia normalizada
            [b, a] = butter(orden, Wn, 'low');

            % Aplicar filtro
            audio_filtrado = filter(b, a, audio);

            fprintf('Parámetros:\n');
            fprintf('  Frecuencia de corte: %d Hz\n', fc);
            fprintf('  Orden del filtro: %d\n', orden);
            fprintf('\nReproduciendo audio filtrado...\n');

            % Reproducir
            player = audioplayer(audio_filtrado, Fs);
            play(player);

            % Preguntar si desea guardar
            guardar = input('\n¿Guardar audio filtrado? (s/n): ', 's');
            if strcmpi(guardar, 's')
                audiowrite('audio_pasa_bajos.wav', audio_filtrado, Fs);
                fprintf('Audio guardado como: audio_pasa_bajos.wav\n');
            end

        case 2
            % ======================================
            % FILTRO PASA-ALTOS
            % ======================================
            fprintf('\n--- Filtro Pasa-Altos ---\n');
            fprintf('Elimina frecuencias inferiores a la frecuencia de corte.\n');
            fprintf('En un sweep, se oirá solo la parte de frecuencias altas.\n\n');

            % Parámetros del filtro - MÁS AGRESIVO
            fc = 2000;  % Frecuencia de corte en Hz (más alta)
            orden = 10;  % Orden más alto = corte más pronunciado

            % Diseño del filtro Butterworth
            Wn = fc / (Fs/2);  % Frecuencia normalizada
            [b, a] = butter(orden, Wn, 'high');

            % Aplicar filtro
            audio_filtrado = filter(b, a, audio);

            fprintf('Parámetros:\n');
            fprintf('  Frecuencia de corte: %d Hz\n', fc);
            fprintf('  Orden del filtro: %d\n', orden);
            fprintf('\nReproduciendo audio filtrado...\n');

            % Reproducir
            player = audioplayer(audio_filtrado, Fs);
            play(player);

            % Preguntar si desea guardar
            guardar = input('\n¿Guardar audio filtrado? (s/n): ', 's');
            if strcmpi(guardar, 's')
                audiowrite('audio_pasa_altos.wav', audio_filtrado, Fs);
                fprintf('Audio guardado como: audio_pasa_altos.wav\n');
            end

        case 3
            % ======================================
            % FILTRO PASA-BANDA
            % ======================================
            fprintf('\n--- Filtro Pasa-Banda ---\n');
            fprintf('Deja pasar solo un rango de frecuencias específico.\n');
            fprintf('En un sweep, se oirá solo la parte media del barrido.\n\n');

            % Parámetros del filtro - MÁS ESTRECHO Y AGRESIVO
            f_low = 1500;  % Frecuencia baja en Hz
            f_high = 3500; % Frecuencia alta en Hz (banda más estrecha)
            orden = 8;     % Orden más alto = corte más pronunciado

            % Diseño del filtro Butterworth
            Wn = [f_low, f_high] / (Fs/2);  % Frecuencias normalizadas
            [b, a] = butter(orden, Wn, 'bandpass');

            % Aplicar filtro
            audio_filtrado = filter(b, a, audio);

            fprintf('Parámetros:\n');
            fprintf('  Frecuencia baja: %d Hz\n', f_low);
            fprintf('  Frecuencia alta: %d Hz\n', f_high);
            fprintf('  Orden del filtro: %d\n', orden);
            fprintf('\nReproduciendo audio filtrado...\n');

            % Reproducir
            player = audioplayer(audio_filtrado, Fs);
            play(player);

            % Preguntar si desea guardar
            guardar = input('\n¿Guardar audio filtrado? (s/n): ', 's');
            if strcmpi(guardar, 's')
                audiowrite('audio_pasa_banda.wav', audio_filtrado, Fs);
                fprintf('Audio guardado como: audio_pasa_banda.wav\n');
            end

        case 4
            % ======================================
            % FILTRO RECHAZA-BANDA (NOTCH)
            % ======================================
            fprintf('\n--- Filtro Rechaza-Banda ---\n');
            fprintf('Elimina un rango específico de frecuencias.\n');
            fprintf('En un sweep, se notará un "hueco" en medio del barrido.\n\n');

            % Parámetros del filtro - MÁS ANCHO Y AGRESIVO
            f_center = 2500; % Frecuencia central en Hz
            bandwidth = 2000; % Ancho de banda en Hz (más ancho)
            orden = 8;       % Orden más alto = corte más pronunciado

            % Calcular frecuencias de corte
            f_low = f_center - bandwidth/2;
            f_high = f_center + bandwidth/2;

            % Diseño del filtro Butterworth
            Wn = [f_low, f_high] / (Fs/2);  % Frecuencias normalizadas
            [b, a] = butter(orden, Wn, 'stop');

            % Aplicar filtro
            audio_filtrado = filter(b, a, audio);

            fprintf('Parámetros:\n');
            fprintf('  Frecuencia central: %d Hz\n', f_center);
            fprintf('  Ancho de banda: %d Hz\n', bandwidth);
            fprintf('  Orden del filtro: %d\n', orden);
            fprintf('\nReproduciendo audio filtrado...\n');

            % Reproducir
            player = audioplayer(audio_filtrado, Fs);
            play(player);

            % Preguntar si desea guardar
            guardar = input('\n¿Guardar audio filtrado? (s/n): ', 's');
            if strcmpi(guardar, 's')
                audiowrite('audio_rechaza_banda.wav', audio_filtrado, Fs);
                fprintf('Audio guardado como: audio_rechaza_banda.wav\n');
            end

        case 5
            % ======================================
            % FILTRO DE ECUALIZACIÓN (REALCE DE MEDIOS)
            % ======================================
            fprintf('\n--- Filtro de Ecualización ---\n');
            fprintf('Realza las frecuencias medias.\n');
            fprintf('En un sweep, se oirá más fuerte la parte media.\n\n');

            % Crear filtro pasa-banda para medios - MÁS AGRESIVO
            f_low = 1500;
            f_high = 3500;
            orden = 8;

            Wn = [f_low, f_high] / (Fs/2);
            [b, a] = butter(orden, Wn, 'bandpass');

            % Extraer frecuencias medias
            medios = filter(b, a, audio);

            % Mezclar con original (realce MÁS FUERTE)
            ganancia = 3.0;  % Factor de realce aumentado
            audio_filtrado = audio + ganancia * medios;

            % Normalizar para evitar clipping
            audio_filtrado = audio_filtrado / max(abs(audio_filtrado));

            fprintf('Parámetros:\n');
            fprintf('  Rango de frecuencias realzadas: %d - %d Hz\n', f_low, f_high);
            fprintf('  Ganancia de realce: %.1fx\n', ganancia);
            fprintf('  Orden del filtro: %d\n', orden);
            fprintf('\nReproduciendo audio filtrado...\n');

            % Reproducir
            player = audioplayer(audio_filtrado, Fs);
            play(player);

            % Preguntar si desea guardar
            guardar = input('\n¿Guardar audio filtrado? (s/n): ', 's');
            if strcmpi(guardar, 's')
                audiowrite('audio_ecualizacion.wav', audio_filtrado, Fs);
                fprintf('Audio guardado como: audio_ecualizacion.wav\n');
            end

        case 6
            % ======================================
            % REPRODUCIR AUDIO ORIGINAL
            % ======================================
            fprintf('\n--- Audio Original ---\n');
            fprintf('Reproduciendo audio sin filtros...\n');

            player = audioplayer(audio, Fs);
            play(player);

        case 7
            % ======================================
            % ANÁLISIS ESPECTRAL COMPARATIVO
            % ======================================
            fprintf('\n--- Análisis Espectral ---\n');
            fprintf('Generando gráficas comparativas...\n\n');

            % Crear los 5 filtros (con parámetros actualizados)
            % 1. Pasa-bajos
            [b1, a1] = butter(10, 2000/(Fs/2), 'low');
            audio_pb = filter(b1, a1, audio);

            % 2. Pasa-altos
            [b2, a2] = butter(10, 2000/(Fs/2), 'high');
            audio_pa = filter(b2, a2, audio);

            % 3. Pasa-banda
            [b3, a3] = butter(8, [1500, 3500]/(Fs/2), 'bandpass');
            audio_band = filter(b3, a3, audio);

            % 4. Rechaza-banda
            [b4, a4] = butter(8, [1500, 3500]/(Fs/2), 'stop');
            audio_notch = filter(b4, a4, audio);

            % 5. Ecualización
            [b5, a5] = butter(8, [1500, 3500]/(Fs/2), 'bandpass');
            medios = filter(b5, a5, audio);
            audio_eq = audio + 3.0 * medios;
            audio_eq = audio_eq / max(abs(audio_eq));

            % FFT del audio original
            L = length(audio);
            NFFT = 2^nextpow2(L);
            f = Fs/2 * linspace(0, 1, NFFT/2+1);

            Y_orig = fft(audio, NFFT)/L;
            Y_pb = fft(audio_pb, NFFT)/L;
            Y_pa = fft(audio_pa, NFFT)/L;
            Y_band = fft(audio_band, NFFT)/L;
            Y_notch = fft(audio_notch, NFFT)/L;
            Y_eq = fft(audio_eq, NFFT)/L;

            % Crear figura con subplots
            figure('Position', [100, 100, 1200, 800]);

            % Original
            subplot(3, 2, 1);
            plot(f, 2*abs(Y_orig(1:NFFT/2+1)));
            title('Espectro - Audio Original');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 5000]);

            % Pasa-bajos
            subplot(3, 2, 2);
            plot(f, 2*abs(Y_pb(1:NFFT/2+1)));
            title('Espectro - Filtro Pasa-Bajos (fc=2000Hz, orden=10)');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 5000]);

            % Pasa-altos
            subplot(3, 2, 3);
            plot(f, 2*abs(Y_pa(1:NFFT/2+1)));
            title('Espectro - Filtro Pasa-Altos (fc=2000Hz, orden=10)');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 5000]);

            % Pasa-banda
            subplot(3, 2, 4);
            plot(f, 2*abs(Y_band(1:NFFT/2+1)));
            title('Espectro - Filtro Pasa-Banda (1500-3500Hz, orden=8)');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 5000]);

            % Rechaza-banda
            subplot(3, 2, 5);
            plot(f, 2*abs(Y_notch(1:NFFT/2+1)));
            title('Espectro - Filtro Rechaza-Banda (1500-3500Hz, orden=8)');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 5000]);

            % Ecualización
            subplot(3, 2, 6);
            plot(f, 2*abs(Y_eq(1:NFFT/2+1)));
            title('Espectro - Ecualización (Realce 1500-3500Hz, 3x)');
            xlabel('Frecuencia (Hz)');
            ylabel('Magnitud');
            grid on;
            xlim([0, 5000]);

            fprintf('Gráficas generadas!\n');

        otherwise
            fprintf('\nOpción inválida. Por favor, selecciona una opción del 0 al 7.\n');
    end

    fprintf('\n');
    input('Presiona Enter para continuar...');
    clc;
end
