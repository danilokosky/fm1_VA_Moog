// ============================================================================
// M-VAVE FM-1: Virtual Analog Engine em C Puro (Moog / Poly D)
// Arquivo: app/synth/moog_engine.h
// ============================================================================

#ifndef MOOG_ENGINE_H
#define MOOG_ENGINE_H

#include <stdint.h>

// Parâmetros de cada oscilador
typedef struct {
    float volume;
    int semi_offset;
    float detune_cents;
    int waveform;       // 0 = Sawtooth, 1 = Pulse
    float pulse_width;
} OscillatorParams;

// Estado e parâmetros do sintetizador Moog
typedef struct {
    float master_volume;
    float cutoff;
    float resonance;
    OscillatorParams osc[3];

    // Estados internos
    float sample_rate;
    float base_freq;
    int note_active;
    float phase[3];
    float phase_increment[3];
    float stage[4];
    float amp_env;
} MoogEngine;

// Funções públicas em C
void moog_init(MoogEngine *engine, float sample_rate);
void moog_note_on(MoogEngine *engine, int note, float velocity);
void moog_note_off(MoogEngine *engine);
void moog_process_buffer(MoogEngine *engine, float *buffer, int num_samples);

#endif // MOOG_ENGINE_H
