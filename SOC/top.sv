module top (
    input          CLOCK_50,
    input  [3:0]   KEY,     // KEY[0] = reset

    // I2C hacia el WM8731
    inout          AUD_I2C_SDAT,
    output         AUD_I2C_SCLK,

    // Audio CODEC – I2S signals
    input          AUD_ADCDAT,
    inout          AUD_ADCLRCK,
    inout          AUD_DACLRCK,
    output         AUD_DACDAT,
    inout          AUD_BCLK
);

    wire reset_n = KEY[0];

    // Instancia del sistema
    SOC u0 (
        .clk_clk          (CLOCK_50),
        .reset_reset_n    (reset_n),

        // I2C
        .av_i2c_SDAT      (AUD_I2C_SDAT),
        .av_i2c_SCLK      (AUD_I2C_SCLK),

        // I2S Audio
        .aud_i2s_ADCDAT   (AUD_ADCDAT),
        .aud_i2s_ADCLRCK  (AUD_ADCLRCK),
        .aud_i2s_BCLK     (AUD_BCLK),
        .aud_i2s_DACDAT   (AUD_DACDAT),
        .aud_i2s_DACLRCK  (AUD_DACLRCK)
    );

endmodule
