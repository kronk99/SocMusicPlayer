#include "filter_stream.h"

/*************************************
 * LOW-PASS FILTER (streaming)
 *
 * y[n] = y[n-1] + alpha * (x[n] - y[n-1])
 *************************************/
void lpf_init(lpf_ctx_t *ctx, float alpha)
{
    ctx->alpha = alpha;
    ctx->y_prev = 0.0f;
}

float lpf_process(lpf_ctx_t *ctx, float x)
{
    float y = ctx->y_prev + ctx->alpha * (x - ctx->y_prev);
    ctx->y_prev = y;
    return y;
}


/*************************************
 * HIGH-PASS FILTER (streaming)
 *
 * y[n] = alpha * ( y[n-1] + x[n] - x[n-1] )
 *************************************/
void hpf_init(hpf_ctx_t *ctx, float alpha)
{
    ctx->alpha  = alpha;
    ctx->y_prev = 0.0f;
    ctx->x_prev = 0.0f;
}

float hpf_process(hpf_ctx_t *ctx, float x)
{
    float y = ctx->alpha * (ctx->y_prev + x - ctx->x_prev);

    ctx->y_prev = y;
    ctx->x_prev = x;

    return y;
}


/*************************************
 * BAND-PASS (internal helper)
 *
 * bp[n] = beta * ( bp[n-1] + alpha * (x[n] - x[n-1]) )
 *************************************/
static inline float bandpass_process(notch_ctx_t *ctx, float x)
{
    float bp = ctx->beta * (ctx->bp_prev + ctx->alpha * (x - ctx->x_prev));

    ctx->bp_prev = bp;
    ctx->x_prev  = x;

    return bp;
}


/*************************************
 * NOTCH FILTER (streaming)
 *
 * notch = input - bandpass(input)
 *************************************/
void notch_init(notch_ctx_t *ctx, float alpha, float beta)
{
    ctx->alpha  = alpha;
    ctx->beta   = beta;
    ctx->bp_prev = 0.0f;
    ctx->x_prev  = 0.0f;
}

float notch_process(notch_ctx_t *ctx, float x)
{
    float bp = bandpass_process(ctx, x);
    return x - bp;
}
