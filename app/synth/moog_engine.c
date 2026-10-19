// ============================================================================
// M-VAVE FM-1: Virtual Analog Engine em C Puro (Moog / Poly D)
// Arquivo: app/synth/moog_engine.c
// ============================================================================

#include "moog_engine.h"
#include <math.h>

static void update_frequencies(MoogEngine *engine) {
    for (int i = 0; i < 3; i++) {
        float total_shift = engine->osc[i].semi_offset + (engine->osc[i].detune_cents / 100.0f);
        float osc_freq = engine->base_freq * powf(2.0f, total_shift / 12.0f);
        engine->phase_increment[i] = osc_freq / engine->sample_rate;
    }
}

void moog_init(MoogEngine *engine, float sample_rate) {
    engine->sample_rate = sample_rate > 0.0f ? sample_rate : 44100.0f;
    engine->master_volume = 0.8f;
    engine->cutoff = 0.6f;
    engine->resonance = 0.3f;
    engine->base_freq = 440.0f;
    engine->note_active = 0;
    engine->amp_env = 0.0f;

    for (int i = 0; i < 4; i++) engine->stage[i] = 0.0f;
    for (int i = 0; i < 3; i++) engine->phase[i] = 0.0f;

    // Configuração estilo Minimoog:
    // OSC 1: Dente de serra principal (-5 cents)
    engine->osc[0].waveform = 0;
    engine->osc[0].volume = 0.8f;
    engine->osc[0].semi_offset = 0;
    engine->osc[0].detune_cents = -5.0f;
    engine->osc[0].pulse_width = 0.5f;

    // OSC 2: Dente de serra secundário (+5 cents)
    engine->osc[1].waveform = 0;
    engine->osc[1].volume = 0.8f;
    engine->osc[1].semi_offset = 0;
    engine->osc[1].detune_cents = +5.0f;
    engine->osc[1].pulse_width = 0.5f;

    // OSC 3: Sub-oitava (-12 semitons, onda quadrada)
    engine->osc[2].waveform = 1;
    engine->osc[2].volume = 0.6f;
    engine->osc[2].semi_offset = -12;
    engine->osc[2].detune_cents = 0.0f;
    engine->osc[2].pulse_width = 0.5f;

    update_frequencies(engine);
}

void moog_note_on(MoogEngine *engine, int note, float velocity) {
    (void)velocity;
    engine->base_freq = 440.0f * powf(2.0f, ((float)note - 69.0f) / 12.0f);
    update_frequencies(engine);
    engine->note_active = 1;
}

void moog_note_off(MoogEngine *engine) {
    engine->note_active = 0;
}

void moog_process_buffer(MoogEngine *engine, float *buffer, int num_samples) {
    for (int sample = 0; sample < num_samples; sample++) {
        if (!engine->note_active && engine->amp_env < 0.0001f) {
            buffer[sample] = 0.0f;
            continue;
        }

        // 1. Envelope de amplitude
        float target_amp = engine->note_active ? 1.0f : 0.0f;
        engine->amp_env += (target_amp - engine->amp_env) * (engine->note_active ? 0.01f : 0.001f);

        // 2. Mistura dos 3 osciladores
        float mixed = 0.0f;
        for (int i = 0; i < 3; i++) {
            engine->phase[i] += engine->phase_increment[i];
            if (engine->phase[i] >= 1.0f) engine->phase[i] -= 1.0f;

            float wave = 0.0f;
            if (engine->osc[i].waveform == 0) {
                wave = (2.0f * engine->phase[i]) - 1.0f; // Sawtooth
            } else {
                wave = (engine->phase[i] < engine->osc[i].pulse_width) ? 1.0f : -1.0f; // Pulse
            }
            mixed += wave * engine->osc[i].volume;
        }
        mixed *= 0.33f;

        // 3. Filtro Moog Ladder (4 Polos)
        float f = engine->cutoff * 0.95f;
        float k = engine->resonance * 3.8f;

        float input = mixed - k * engine->stage[3];
        engine->stage[0] += f * (tanhf(input) - tanhf(engine->stage[0]));
        engine->stage[1] += f * (tanhf(engine->stage[0]) - tanhf(engine->stage[1]));
        engine->stage[2] += f * (tanhf(engine->stage[1]) - tanhf(engine->stage[2]));
        engine->stage[3] += f * (tanhf(engine->stage[2]) - tanhf(engine->stage[3]));

        // 4. Saída final com volume master
        buffer[sample] = engine->stage[3] * engine->amp_env * engine->master_volume;
    }
}
