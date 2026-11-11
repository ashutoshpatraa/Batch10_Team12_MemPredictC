#ifndef MEMPREDICTC_UI_H
#define MEMPREDICTC_UI_H

/*
 * Starts the native Windows dashboard.
 * Returns non-zero after a successful window initialization, or zero when
 * the caller should use the terminal fallback.
 */
int ui_run(void);

#endif
