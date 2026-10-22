#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/events/keycode_state_changed.h>
#include <zmk/event_manager.h>

#include <prospector/brightness.h>

LOG_MODULE_REGISTER(prospector_screen_idle, CONFIG_ZMK_LOG_LEVEL);

#if CONFIG_PROSPECTOR_SCREEN_IDLE_TIMEOUT_S > 0

static void screen_idle_timeout_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(screen_idle_timeout_work, screen_idle_timeout_handler);

static void screen_idle_timeout_handler(struct k_work *work) {
    int ret = prospector_display_set_idle(true);
    if (ret < 0) {
        LOG_ERR("Failed to turn off the display backlight (%d)", ret);
    }
}

static int screen_idle_keycode_listener(const zmk_event_t *eh) {
    struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev == NULL || !ev->state) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    k_work_reschedule(&screen_idle_timeout_work,
                      K_SECONDS(CONFIG_PROSPECTOR_SCREEN_IDLE_TIMEOUT_S));

    int ret = prospector_display_set_idle(false);
    if (ret < 0) {
        LOG_ERR("Failed to restore the display backlight (%d)", ret);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(prospector_screen_idle, screen_idle_keycode_listener);
ZMK_SUBSCRIPTION(prospector_screen_idle, zmk_keycode_state_changed);

static int screen_idle_init(void) {
    k_work_reschedule(&screen_idle_timeout_work,
                      K_SECONDS(CONFIG_PROSPECTOR_SCREEN_IDLE_TIMEOUT_S));
    return 0;
}

SYS_INIT(screen_idle_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#endif
