#ifndef FILTER_STREAM_H
#define FILTER_STREAM_H

#include <stdint.h>

/*******************************
 * LOW-PASS FILTER CONTEXT
 *******************************/
typedef struct {
    float alpha;
    float y_prev;
} lpf_ctx_t;

/*******************************
 * HIGH-PASS FILTER CONTEXT
 *******************************/
typedef struct {
    float alpha;
    float y_prev;
    float x_prev;
} hpf_ctx_t;

/*******************************
 * NOTCH (BAND-STOP) FILTER CONTEXT
 *
 * Uses an internal band-pass to construct the notch.
 *******************************/
typedef struct {
    float alpha;
    float beta;

    float bp_prev;   // previous output of band-pass
    float x_prev;    // previous input
} notch_ctx_t;


/***************
 * INITIALIZERS
 ***************/
void lpf_init(lpf_ctx_t *ctx, float alpha);
void hpf_init(hpf_ctx_t *ctx, float alpha);
void notch_init(notch_ctx_t *ctx, float alpha, float beta);


/***********************
 * PROCESSING FUNCTIONS
 * (1 sample at a time)
 ***********************/
float lpf_process(lpf_ctx_t *ctx, float x);
float hpf_process(hpf_ctx_t *ctx, float x);
float notch_process(notch_ctx_t *ctx, float x);

#endif
