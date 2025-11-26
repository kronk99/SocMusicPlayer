
module SOC (
	clk_clk,
	reset_reset_n,
	av_i2c_SDAT,
	av_i2c_SCLK,
	aud_i2s_ADCDAT,
	aud_i2s_ADCLRCK,
	aud_i2s_BCLK,
	aud_i2s_DACDAT,
	aud_i2s_DACLRCK);	

	input		clk_clk;
	input		reset_reset_n;
	inout		av_i2c_SDAT;
	output		av_i2c_SCLK;
	input		aud_i2s_ADCDAT;
	input		aud_i2s_ADCLRCK;
	input		aud_i2s_BCLK;
	output		aud_i2s_DACDAT;
	input		aud_i2s_DACLRCK;
endmodule
