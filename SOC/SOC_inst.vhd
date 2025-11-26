	component SOC is
		port (
			clk_clk         : in    std_logic := 'X'; -- clk
			reset_reset_n   : in    std_logic := 'X'; -- reset_n
			av_i2c_SDAT     : inout std_logic := 'X'; -- SDAT
			av_i2c_SCLK     : out   std_logic;        -- SCLK
			aud_i2s_ADCDAT  : in    std_logic := 'X'; -- ADCDAT
			aud_i2s_ADCLRCK : in    std_logic := 'X'; -- ADCLRCK
			aud_i2s_BCLK    : in    std_logic := 'X'; -- BCLK
			aud_i2s_DACDAT  : out   std_logic;        -- DACDAT
			aud_i2s_DACLRCK : in    std_logic := 'X'  -- DACLRCK
		);
	end component SOC;

	u0 : component SOC
		port map (
			clk_clk         => CONNECTED_TO_clk_clk,         --     clk.clk
			reset_reset_n   => CONNECTED_TO_reset_reset_n,   --   reset.reset_n
			av_i2c_SDAT     => CONNECTED_TO_av_i2c_SDAT,     --  av_i2c.SDAT
			av_i2c_SCLK     => CONNECTED_TO_av_i2c_SCLK,     --        .SCLK
			aud_i2s_ADCDAT  => CONNECTED_TO_aud_i2s_ADCDAT,  -- aud_i2s.ADCDAT
			aud_i2s_ADCLRCK => CONNECTED_TO_aud_i2s_ADCLRCK, --        .ADCLRCK
			aud_i2s_BCLK    => CONNECTED_TO_aud_i2s_BCLK,    --        .BCLK
			aud_i2s_DACDAT  => CONNECTED_TO_aud_i2s_DACDAT,  --        .DACDAT
			aud_i2s_DACLRCK => CONNECTED_TO_aud_i2s_DACLRCK  --        .DACLRCK
		);

