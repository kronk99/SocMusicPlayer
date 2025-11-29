import { useEffect, useState } from "react";

const API_URL = "http://192.168.18.120:8080/api/track";
function formatTime(totalSeconds) {
  if (!Number.isFinite(totalSeconds)) return "--:--";
  const sec = Math.max(0, Math.floor(totalSeconds));
  const minutes = Math.floor(sec / 60);
  const seconds = sec % 60;
  return `${String(minutes).padStart(2, "0")}:${String(seconds).padStart(2, "0")}`;
}

// --- Componente para inyectar los Keyframes de animación ---
// Esto es necesario porque las animaciones CSS (keyframes) no se pueden definir
// fácilmente solo con objetos de estilo en línea.
const AnimationStyles = () => (
  <style dangerouslySetInnerHTML={{ __html: `
    @keyframes spinRecord {
      from { transform: rotate(0deg); }
      to { transform: rotate(360deg); }
    }
    /* Esta clase se aplicará al disco */
    .vinyl-spinning {
      animation: spinRecord 5s linear infinite;
      will-change: transform; /* Optimización de rendimiento */
    }
  `}} />
);


export default function Home() {
  const [track, setTrack] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState("");

  useEffect(() => {
    let isMounted = true;

    const fetchTrack = async () => {
      try {
        const res = await fetch(API_URL);
        if (!res.ok) {
          throw new Error(`HTTP ${res.status}`);
        }
        const data = await res.json();
        if (isMounted) {
          setTrack(data);
          setError("");
          setLoading(false);
        }
      } catch (err) {
        if (isMounted) {
          console.error("Error al obtener metadatos:", err);
          setError("No se pudo conectar con el servidor.");
          setLoading(false);
        }
      }
    };

    fetchTrack();
    const id = setInterval(fetchTrack, 1000);

    return () => {
      isMounted = false;
      clearInterval(id);
    };
  }, []);

  // --- Renderizado de Estados de Carga / Error ---

  if (loading) {
    return (
      <main style={styles.main}>
        <div style={styles.loadingContainer}>
          <div style={styles.spinner}></div>
          <p>Conectando...</p>
        </div>
      </main>
    );
  }

  if (error) {
    return (
      <main style={styles.main}>
        <div style={styles.container}>
            <h2 style={{...styles.title, textAlign: 'center'}}>⚠️ ERROR</h2>
            <p style={{ color: "#ff5555", marginTop: '10px', textAlign: 'center' }}>{error}</p>
        </div>
      </main>
    );
  }

  if (!track) {
    return (
      <main style={styles.main}>
        <div style={styles.container}>
            <p>Esperando pista...</p>
        </div>
      </main>
    );
  }

  // --- Renderizado Principal ---

  const current = track.current_seconds ?? 0;
  const total = track.duration_seconds ?? 0;
  const progress = total > 0 ? Math.min(100, (current / total) * 100) : 0;

  return (
    <main style={styles.main}>
      {/* Inyectamos los estilos de animación aquí */}
      <AnimationStyles />

      <div style={styles.container}>
        
        {/* Header */}
        <div style={styles.header}>
            <span style={styles.headerText}>REPRODUCTOR DE AUDIO</span>
            <span style={styles.headerSubText}>{track.album || "Single"}</span>
        </div>

        {/* Imagen Central: DISCO DE VINILO ROTATORIO */}
        <div style={styles.artContainer}>
            {/* Aplicamos la clase 'vinyl-spinning' definida en AnimationStyles.
                Si la música estuviera en pausa (no tenemos ese estado ahora mismo),
                podrías quitar esta clase condicionalmente.
            */}
            <div style={styles.vinylRecord} className="vinyl-spinning">
                {/* La etiqueta central del disco */}
                <div style={styles.vinylLabel}>
                    <p style={styles.vinylLabelText}>
                        Proyecto 2<br />Embedded Systems
                    </p>
                    {/* Agujero central pequeño */}
                    <div style={styles.vinylHole}></div>
                </div>
            </div>
        </div>

        {/* Info de la canción */}
        <div style={styles.trackInfo}>
            <div style={styles.titlesWrapper}>
                <h1 style={styles.songTitle}>{track.title || "Sin título"}</h1>
                <p style={styles.artistName}>{track.artist || "Artista Desconocido"}</p>
            </div>
        </div>

        {/* Barra de Progreso */}
        <div style={styles.progressContainer}>
          <div style={styles.progressBarBg}>
            <div style={{ ...styles.progressBarFill, width: `${progress}%` }}>
                <div style={styles.progressHandle}></div>
            </div>
          </div>
          <div style={styles.timeRow}>
            <span>{formatTime(current)}</span>
            <span>{formatTime(total)}</span>
          </div>
        </div>

        {/* Datos técnicos */}
        <div style={styles.techSpecs}>
            <div style={styles.techPill}>SAMPLE: {track.sample_rate} HZ</div>
            <div style={styles.techPill}>BITS: {track.bits_per_sample}</div>
            <div style={styles.techPill}>CH: {track.channels}</div>
        </div>
      </div>
    </main>
  );
}

// --- Estilos CSS en Objeto JS ---

const styles = {
  main: {
    minHeight: "100vh",
    display: "flex",
    alignItems: "center",
    justifyContent: "center",
    background: "linear-gradient(180deg, #404040 0%, #121212 100%)", 
    fontFamily: "'Circular Std', 'Helvetica Neue', Helvetica, Arial, sans-serif",
    color: "#FFFFFF",
    padding: "20px",
    overflow: "hidden" // Evitar scrollbars si la animación sobresale un pixel
  },
  container: {
    width: "100%",
    maxWidth: "400px",
    display: "flex",
    flexDirection: "column",
    gap: "24px",
    zIndex: 1,
  },
  header: {
    textAlign: "center",
    marginBottom: "5px",
    opacity: 0.8,
  },
  headerText: {
    fontSize: "0.7rem",
    letterSpacing: "1px",
    fontWeight: "600",
    display: "block",
    marginBottom: "4px",
  },
  headerSubText: {
    fontSize: "0.8rem",
    fontWeight: "bold",
  },
  // --- ESTILOS DEL VINILO ---
  artContainer: {
    display: "flex",
    justifyContent: "center",
    alignItems: "center",
    margin: "10px 0 30px 0",
    // Un poco de perspectiva para que el giro se vea mejor (opcional)
    perspective: "1000px", 
  },
  vinylRecord: {
    width: "280px",
    height: "280px",
    borderRadius: "50%",
    // Un gradiente radial repetido crea el efecto de los surcos del vinilo
    background: "repeating-radial-gradient(circle at center, #1a1a1a 0, #1a1a1a 2px, #0a0a0a 3px, #0a0a0a 4px)",
    boxShadow: "0 10px 30px rgba(0,0,0,0.5), inset 0 0 10px rgba(255,255,255,0.1)", // Sombra externa y brillo interno sutil
    display: "flex",
    alignItems: "center",
    justifyContent: "center",
    position: "relative",
    border: "3px solid #050505"
  },
  vinylLabel: {
    width: "110px",
    height: "110px",
    borderRadius: "50%",
    // Color de la etiqueta ( Rojo clásico )
    background: "#d32f2f", 
    display: "flex",
    alignItems: "center",
    justifyContent: "center",
    textAlign: "center",
    border: "4px solid #1a1a1a", // El borde oscuro entre la etiqueta y el disco
    boxShadow: "inset 0 0 15px rgba(0,0,0,0.3)",
    position: "relative",
  },
  vinylLabelText: {
    color: "#fce7e7", // Color de texto claro que contraste con el rojo
    fontWeight: "800",
    fontSize: "0.6rem",
    lineHeight: "1.3",
    textTransform: "uppercase",
    letterSpacing: "0.5px",
    margin: 0,
    padding: "5px",
  },
  vinylHole: {
    position: "absolute",
    width: "12px",
    height: "12px",
    background: "#121212", // El color del fondo principal para parecer un agujero real
    borderRadius: "50%",
    border: "1px solid rgba(255,255,255,0.1)"
  },
  // --------------------------

  trackInfo: {
    display: "flex",
    justifyContent: "space-between",
    alignItems: "flex-end",
  },
  titlesWrapper: {
    display: "flex",
    flexDirection: "column",
    gap: "4px",
    width: "100%",
    textAlign: "center", // Títulos centrados 
    
  },
  songTitle: {
    fontSize: "1.5rem",
    fontWeight: "700",
    margin: 0,
    lineHeight: "1.2",
    whiteSpace: "nowrap",
    overflow: "hidden",
    textOverflow: "ellipsis",
  },
  artistName: {
    fontSize: "1rem",
    color: "#b3b3b3",
    margin: 0,
    fontWeight: "400",
  },
  progressContainer: {
    width: "100%",
    marginTop: "10px",
  },
  progressBarBg: {
    width: "100%",
    height: "4px",
    background: "#535353",
    borderRadius: "2px",
    position: "relative",
    overflow: "visible",
  },
  progressBarFill: {
    height: "100%",
    background: "#1db954",
    borderRadius: "2px",
    position: "relative",
    maxWidth: "100%",
    transition: "width 0.1s linear",
  },
  progressHandle: {
    width: "12px",
    height: "12px",
    background: "#fff",
    borderRadius: "50%",
    position: "absolute",
    right: "-6px",
    top: "-4px",
    boxShadow: "0 2px 4px rgba(0,0,0,0.5)",
  },
  timeRow: {
    display: "flex",
    justifyContent: "space-between",
    fontSize: "0.75rem",
    color: "#b3b3b3",
    marginTop: "8px",
    fontVariantNumeric: "tabular-nums",
  },
  techSpecs: {
    marginTop: "10px",
    display: "flex",
    flexWrap: "wrap",
    gap: "8px",
    justifyContent: "center",
    opacity: 0.5,
  },
  techPill: {
    fontSize: "0.65rem",
    background: "rgba(255,255,255,0.1)",
    padding: "4px 8px",
    borderRadius: "4px",
    color: "#ccc",
  },
  // Loading styles
  loadingContainer: {
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
    color: "#1db954",
  },
  spinner: {
    width: "40px",
    height: "40px",
    border: "4px solid rgba(255,255,255,0.1)",
    borderLeftColor: "#1db954",
    borderRadius: "50%",
    // Aquí sí podemos usar la animación en línea porque es simple
    animation: "spin 1s linear infinite", 
    marginBottom: "16px",
  },
};